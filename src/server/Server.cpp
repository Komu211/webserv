#include "Server.hpp"

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

    std::unordered_map<int, std::string> clientRequests;

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
                acceptNewConnections(serverFd, clientRequests);
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << e.what() << '\n';
            }
        }

        // Stage 2: Read from clients
        for (int clientFd : _pollManager.getReadableClientSockets())
        {
            try
            {
                readFromClient(clientFd, clientRequests, clientsToRemove);
                // TODO: Add check here (or somewhere) for response is ready to be sent back
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << "Error reading from client " << clientFd << ": " << e.what() << '\n';
                clientsToRemove.push_back(clientFd);
            }
        }

        // Stage 3: Write responses to clients
        for (int clientFd : _pollManager.getWritableClientSockets())
        {
            try
            {
                writeResponseToClient(clientFd, clientRequests);
                // No need to close the connection unless request has `Connection: close` header (HTTP/1.1)
                // TODO: add check for `Connection: close` header then conditionally add to clientsToRemove
                // clientsToRemove.push_back(clientFd);
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << "Error writing to client " << clientFd << ": " << e.what() << '\n';
                clientsToRemove.push_back(clientFd);
            }
        }

        // Clean up closed connections
        for (int fd : clientsToRemove)
        {
            _pollManager.removeSocket(fd);
            clientRequests.erase(fd);
            close(fd);
        }
    }
    std::cout << "Server successfully stopped. Goodbye!" << '\n';
}

void Server::acceptNewConnections(int serverFd, std::unordered_map<int, std::string> &clientRequests)
{
    const int clientFd = accept(serverFd, nullptr, nullptr);
    if (clientFd >= 0)
    {
        std::cout << "Accepted new connection via: \n" << *(_sockets[serverFd]) << '\n';
        _pollManager.addClientSocket(clientFd); // POLLOUT should only be registered after a client sends a request and a response is ready to be sent back
        clientRequests[clientFd] = "";
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

void Server::readFromClient(int clientFd, std::unordered_map<int, std::string> &clientRequests, std::vector<int> &clientsToRemove)
{
    char    buffer[1024]; // ! Requests can be larger than 1024 bytes; read in a loop
    ssize_t bytesRead = read(clientFd, buffer, sizeof(buffer) - 1);

    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        clientRequests[clientFd] += buffer;

        // Put the received request into a file - TODO: replace with a proper request parser
        if (std::ofstream requestFile("request_" + std::to_string(clientFd) + ".txt"); requestFile.is_open())
        {
            requestFile << clientRequests[clientFd];
            requestFile.close();
        }
        else
        {
            throw std::runtime_error("Error opening file to write request: " + std::string(strerror(errno)));
        }
        // TODO: Add check here (or somewhere) for response is ready to be sent
        _pollManager.updateEvents(clientFd, POLLOUT); // Register interest in writing back to client assuming that response is ready
    }
    else if (bytesRead == 0)
    {
        // Client closed connection
        std::cout << "Client " << clientFd << " closed connection" << '\n';
        clientsToRemove.push_back(clientFd);
    }
    else
    {
        // Error reading from client
        throw std::runtime_error("Error reading from client " + std::to_string(clientFd) + ": " + strerror(errno));
    }
}
void Server::writeResponseToClient(int clientFd, std::unordered_map<int, std::string> &clientRequests)
{
    if (!clientRequests[clientFd].empty())
    {
        std::string response = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/plain\r\n"
                               "Content-Length: 13\r\n"
                               "\r\n"
                               "Hello, world!";

        if (send(clientFd, response.c_str(), response.size(), 0) > 0)
        {
            std::cout << "Sent response to client " << clientFd << '\n';
        }
        else
        {
            throw std::runtime_error("Error sending response to client " + std::to_string(clientFd) + ": " + strerror(errno));
        }
    }
    _pollManager.removeEvents(clientFd, POLLOUT); // Stop monitoring for writing until new request arrives / new response is ready
}
