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
// std::vector<ServerConfig> Server::get_configs() const
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

    std::unordered_map<int, std::string> partialRequests;
    std::unordered_map<int, std::unique_ptr<HTTPRequest>> parsedRequests;
    std::unordered_map<int, PendingResponse> pendingResponses;

    while (g_shutdownServer == 0)
    {
        /*
         *TODO:
         * 1. Poll for events on the watched FDs ✅
         * 2. Accept new connections ✅
         * 3. Read requests from clients ✅
         * 4. Read available internal files
         * 5. Send responses to clients ✅
         * 6. Close finished and timeout connections ✅
         */

        const int        pollResult = poll(_pollManager.data(), _pollManager.size(), -1);
        std::vector<int> clientsToRemove;

        if (pollResult < 0)
        {
            // poll() was interrupted by a signal. Check g_shutdownServer flag in loop condition
            if (errno == EINTR)
                continue;

            std::cerr << "Poll error: " << strerror(errno) << '\n';
            continue;
        }
        if (pollResult == 0)
        {
            std::cout << "No events occurred within the timeout." << '\n';
            continue;
        }

        // Stage 1: Handle new connections
        for (const int serverFd : _pollManager.getReadableServerSockets())
        {
            try
            {
                acceptNewConnections(serverFd, partialRequests);
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << e.what() << '\n';
            }
        }

        // Stage 2: Read from clients
        for (int clientFd : _pollManager.getReadableClientSockets())
        {
            std::string currentRequest = partialRequests[clientFd];
            try
            {
                currentRequest = readFromClient(clientFd, currentRequest);
                std::cout << "Received request from client: " << clientFd << '\n';
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << "Error reading from client " << clientFd << ": " << e.what() << '\n';
                clientsToRemove.push_back(clientFd);
                continue;
            }
            if (currentRequest.empty())
                clientsToRemove.push_back(clientFd);
            else if (HTTPRequestParser::isValidRequest(currentRequest))
            {
                try
                {
                    HTTPRequestData data = HTTPRequestParser::parse(currentRequest);
                    parsedRequests[clientFd] = HTTPRequestFactory::createRequest(data);
                    _pollManager.updateEvents(clientFd, POLLOUT);
                }
                catch (const std::runtime_error &e)
                {
                    std::cerr << "Error parsing request: " << e.what() << '\n';
                    clientsToRemove.push_back(clientFd);
                }
            }
            else
                partialRequests[clientFd] = currentRequest;
        }

        // Stage 3: Write responses to clients
        for (int clientFd : _pollManager.getWritableClientSockets())
        {
            if (parsedRequests.find(clientFd) != parsedRequests.end())
            {
                try
                {
                    std::cout << "Sending response to client: " << clientFd << '\n';
                    PendingResponse pendingResponse = {"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\nHello, World!", 0};
                    if (pendingResponses.find(clientFd) != pendingResponses.end())
                        pendingResponse = pendingResponses[clientFd];
                    pendingResponse = writeResponseToClient(clientFd, parsedRequests[clientFd], pendingResponse);
                    if (pendingResponse.response.size() == pendingResponse.sent)
                    {
                        std::cout << "All sent, switch back to listening" << std::endl;
                        pendingResponses.erase(clientFd);
                        _pollManager.updateEvents(clientFd, POLLIN); // Start monitoring for reading new requests
                        _pollManager.removeEvents(clientFd, POLLOUT); // Stop monitoring for writing until new request arrives / new response is ready
                    }
                    else
                        pendingResponses[clientFd] = pendingResponse;
                    // TODO: add check for `Connection: close` header then conditionally add to clientsToRemove
                }
                catch (const std::runtime_error &e)
                {
                    std::cerr << "Error writing to client " << clientFd << ": " << e.what() << '\n';
                    clientsToRemove.push_back(clientFd);
                }
            }
        }

        // Clean up closed connections
        for (int fd : clientsToRemove)
        {
            _pollManager.removeSocket(fd);
            partialRequests.erase(fd);
            parsedRequests.erase(fd);
            close(fd);
        }
    }
    std::cout << "Server successfully stopped. Goodbye!" << '\n';
}

void Server::acceptNewConnections(int serverFd, std::unordered_map<int, std::string> &partialRequests)
{
    const int clientFd = accept(serverFd, nullptr, nullptr);
    if (clientFd >= 0)
    {
        std::cout << "Accepted new connection via: \n" << *(_sockets[serverFd]) << '\n';
        _pollManager.addClientSocket(clientFd);
        partialRequests[clientFd] = "";
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

std::string Server::readFromClient(int clientFd, std::string partial_request)
{
    char        buffer[BUFFER_SIZE];
    ssize_t     bytesRead = read(clientFd, buffer, BUFFER_SIZE - 1);

    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        partial_request += buffer;
        return partial_request;
    }
    else if (bytesRead == 0)
        return ""; // Client has closed connection
    else
    {
        // Error reading from client
        throw std::runtime_error("Error reading from client " + std::to_string(clientFd) + ": " + strerror(errno));
    }
}
PendingResponse Server::writeResponseToClient(int clientFd, std::unique_ptr<HTTPRequest> &clientRequest, PendingResponse &pendingResponses)
{

    (void)clientRequest; // Avoid unused variable warning
    // HTTPResponse = clientRequest->handle();
    // TODO: Create a HTTPResponse class and implement handle() method

    auto responseRemainder = pendingResponses.response.c_str() + pendingResponses.sent;
    auto remainingToSend = pendingResponses.response.size() - pendingResponses.sent;

    // Send response back to client
    ssize_t bytesWritten = write(clientFd, responseRemainder, remainingToSend);
    if (bytesWritten < 0)
        throw std::runtime_error("Error writing to client " + std::to_string(clientFd) + ": " + strerror(errno));
    pendingResponses.sent += bytesWritten;
    return pendingResponses;
}
