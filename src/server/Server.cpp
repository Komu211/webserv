#include "Server.hpp"

#include "HTTPRequest.hpp"
#include "HTTPRequestFactory.hpp"

#include <utility>

Server::Server(std::string configFileName)
    : _global_config{std::move(configFileName)} // Initiate parsing of the config file
{
    // Create listening sockets
    for (const auto &server_config : _global_config.getServerConfigs())
    {
        // Each server_config can be listening on multiple host_port combinations
        for (const auto &addr_info_pair : server_config->getAddrInfoVec())
        {
            // TODO: Add fd to server_name mapping because both host:port (i.e., fd) and server_name must match for request routing
            auto newSocket{std::make_unique<Socket>(addr_info_pair)};
            bool exists = false;
            for (const auto &[_, existingSocket] : _sockets)
            {
                if (*existingSocket == *newSocket)
                    exists = true;
                std::cout << "[warn] conflicting listen on " << existingSocket->get_host() << ':' << existingSocket->get_port() << ", ignored" << '\n';
            }
            if (exists)
                continue;
            try
            {
                newSocket->initSocket();
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << "Error initializing socket: " << e.what() << '\n';
                continue;
            }
            _sockets[newSocket->get_fd()] = std::move(newSocket);
        }
    }
    if (_sockets.empty())
        throw std::runtime_error("No valid listen addresses available. Cannot start server.");
}

// // Not needed + Inefficient: making a copy of vector on every return
// std::vector<ServerConfig> Server::get_configs() const.0
// {
//     return _configs;
// }

void Server::fillPollManager()
{
    for (const auto &[fd, sockPtr] : _sockets)
    {
        _pollManager.addServerSocket(fd);
    }
}

void Server::run()
{
    std::cout << "Server is running..." << '\n';
    std::cout << "Listening on the following sockets:" << '\n';
    for (const auto &[_, sockPtr] : _sockets)
    {
        std::cout << " - " << *sockPtr << '\n';
    }

    while (g_shutdownServer == 0)
    {
        const int   pollResult = poll(_pollManager.data(), _pollManager.size(), -1);

        if (pollResult < 0)
        {
            // poll() was interrupted by a signal. Check g_shutdownServer flag in loop condition
            if (errno == EINTR)
                continue;

            std::cerr << "Poll error: " << strerror(errno) << '\n';
            continue;
        }
        // Stage 1: Accept new connections
        acceptNewConnections();

        // Stage 2: Read from clients
        readFromClients();

        // Stage 3: Write responses to clients
        respondToClients();

        // Clean up closed connections
        closeConnections();
    }
    std::cout << "Server successfully stopped. Goodbye!" << '\n';
}

void Server::acceptNewConnections()
{
    for (const int serverFd : _pollManager.getReadableServerSockets())
    {
        try
        {
            acceptNewConnection(serverFd);
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}

void Server::acceptNewConnection(int serverFd)
{
    const int clientFd = accept(serverFd, nullptr, nullptr);
    if (clientFd >= 0)
    {
        std::cout << "Accepted new connection via: \n" << *(_sockets[serverFd]) << '\n';
        _pollManager.addClientSocket(clientFd);
        _clientData[clientFd] = {"", nullptr, {}};
    }
    else
    {
        // accept returns -1 if there are no more connections to accept; that's not necessarily an error if (errno == EAGAIN or EWOULDBLOCK)
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            std::cout << "No more connections to accept via: \n" << *(_sockets[serverFd]) << '\n';
            return;
        }
        // Throw only if (errno != EAGAIN or EWOULDBLOCK)
        throw std::runtime_error("Error accepting new connection: " + std::string(strerror(errno)));
    }
}
void Server::readFromClients()
{
    for (int clientFd : _pollManager.getReadableClientSockets())
    {
        std::string currentRequest;
        try
        {
            currentRequest = readFromClient(clientFd);
            std::cout << "Received request from client: " << clientFd << '\n';
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << "Error reading from client " << clientFd << ": " << e.what() << '\n';
            _clientsToRemove.insert(clientFd);
            continue;
        }
        if (currentRequest.empty())
            _clientsToRemove.insert(clientFd);
        else if (HTTPRequestParser::isValidRequest(currentRequest))
        {
            try
            {
                HTTPRequestData data = HTTPRequestParser::parse(currentRequest);
                _clientData[clientFd].parsedRequest = HTTPRequestFactory::createRequest(data);
                _pollManager.updateEvents(clientFd, POLLOUT);
                _clientData[clientFd].partialRequest.clear();
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << "Error parsing request: " << e.what() << '\n';
                _clientsToRemove.insert(clientFd);
            }
        }
        else
            _clientData[clientFd].partialRequest = currentRequest;
    }
}

std::string Server::readFromClient(int clientFd)
{
    char          buffer[BUFFER_SIZE];
    const ssize_t bytesRead = read(clientFd, buffer, BUFFER_SIZE - 1);
    std::string   partialRequest = _clientData[clientFd].partialRequest;

    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        partialRequest += buffer;
        return partialRequest;
    }
    else if (bytesRead == 0)
        return ""; // Client has closed connection
    else
        throw std::runtime_error("Error reading from client " + std::to_string(clientFd) + ": " + strerror(errno));
}
void Server::respondToClients()
{
    for (int clientFd : _pollManager.getWritableClientSockets())
    {
        if (_clientData[clientFd].parsedRequest != nullptr)
        {
            try
            {
                respondToClient(clientFd);
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << "Error writing to client " << clientFd << ": " << e.what() << '\n';
                _clientsToRemove.insert(clientFd);
            }
        }
    }
}

void Server::respondToClient(int clientFd)
{
    std::cout << "Sending response to client: " << clientFd << '\n';
    // HTTPResponse = clientRequest->handle();
    // TODO: Create a HTTPResponse class and implement handle() method
    if (_clientData[clientFd].pendingResponse.response.empty())
        _clientData[clientFd].pendingResponse = {
        "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\nHello, World!",
        0};
    auto pendingResponse = writeResponseToClient(clientFd);
    if (pendingResponse.response.size() == pendingResponse.sent)
    {
        std::cout << "All sent, switch back to listening" << std::endl;
        if (_clientData[clientFd].parsedRequest->isCloseConnection())
            _clientsToRemove.insert(clientFd);
        _clientData[clientFd].pendingResponse = {};
        _clientData[clientFd].parsedRequest = nullptr;
        _pollManager.updateEvents(clientFd, POLLIN); // Start monitoring for reading new requests
        _pollManager.removeEvents(clientFd, POLLOUT); // Stop monitoring for writing until new request arrives / new response is ready
    }
    else
        _clientData[clientFd].pendingResponse = pendingResponse;
}

PendingResponse Server::writeResponseToClient(int clientFd)
{
    auto pendingResponses = _clientData[clientFd].pendingResponse;
    auto responseRemainder = pendingResponses.response.c_str() + pendingResponses.sent;
    auto remainingToSend = pendingResponses.response.size() - pendingResponses.sent;

    // Send response back to client
    ssize_t bytesWritten = write(clientFd, responseRemainder, remainingToSend);
    if (bytesWritten < 0)
        throw std::runtime_error("Error writing to client " + std::to_string(clientFd) + ": " + strerror(errno));
    pendingResponses.sent += bytesWritten;
    return pendingResponses;
}

void Server::closeConnections()
{
    for (int fd : _clientsToRemove)
    {
        _pollManager.removeSocket(fd);
        _clientData.erase(fd);
        close(fd);

    }
    _clientsToRemove.clear();
}
