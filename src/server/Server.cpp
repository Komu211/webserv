#include "Server.hpp"

#include <fstream>

Server::Server(std::vector<ServerConfig> configs) : _configs(std::move(configs))
{
    for (const auto &config : _configs)
    {
        auto newSocket = std::make_unique<Socket>(config.getHost(), config.getPort());
        bool exists = false;
        for (const auto &[_, existingSocket] : _sockets)
        {
            if (*existingSocket == *newSocket)
                exists = true;
        }
        if (exists)
            continue;
        try
        {
            newSocket->initSocket();
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << "Error initializing socket: " << e.what() << std::endl;
            continue;
        }
        _sockets[newSocket->get_fd()] = std::move(newSocket);
    }
}

std::vector<ServerConfig> Server::get_configs() const
{
    return _configs;
}
void Server::set_configs(const std::vector<ServerConfig> &configs)
{
    _configs = configs;
}

void Server::fillPollManager()
{
    for (const auto &[fd, sockPtr] : _sockets)
    {
        _pollManager.addServerSocket(fd);
    }
}

void Server::run()
{
    std::cout << "Server is running..." << std::endl;
    std::cout << "Listening on the following sockets:" << std::endl;
    for (const auto &[_, sockPtr] : _sockets)
    {
        std::cout << " - " << *sockPtr << std::endl;
    }

    std::unordered_map<int, std::string> clientRequests;

    while (42)
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

        const int pollResult = poll(_pollManager.data(), _pollManager.size(), -1);
        std::vector<int> clientsToRemove;

        if (pollResult < 0)
        {
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
            continue;
        }
        if (pollResult == 0)
        {
            std::cout << "No events occurred within the timeout." << std::endl;
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
                std::cerr << e.what() << std::endl;
            }
        }

        // Stage 2: Read from clients
        for (int clientFd : _pollManager.getReadableClientSockets())
        {
            try
            {
                readFromClient(clientFd, clientRequests, clientsToRemove);
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << "Error reading from client " << clientFd << ": " << e.what() << std::endl;
                clientsToRemove.push_back(clientFd);
            }
        }

        // Stage 3: Write responses to clients
        for (int clientFd : _pollManager.getWritableClientSockets())
        {
            try
            {
                writeResponseToClient(clientFd, clientRequests);
                clientsToRemove.push_back(clientFd);
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << "Error writing to client " << clientFd << ": " << e.what() << std::endl;
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
}

void Server::acceptNewConnections(int serverFd, std::unordered_map<int, std::string> &clientRequests)
{
    const int clientFd = accept(serverFd, nullptr, nullptr);
    if (clientFd >= 0)
    {
        std::cout << "Accepted new connection via port: \n"
                  << *(_sockets[serverFd]) << std::endl;
        _pollManager.addClientSocket(clientFd);
        clientRequests[clientFd] = "";
    }
    else
    {
        throw std::runtime_error("Error accepting new connection: " + std::string(strerror(errno)));
    }
}

void Server::readFromClient(int clientFd, std::unordered_map<int, std::string> &clientRequests, std::vector<int> &clientsToRemove)
{
    char    buffer[1024];
    ssize_t bytesRead = read(clientFd, buffer, sizeof(buffer) - 1);

    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        clientRequests[clientFd] += buffer;

        // Put the recieved request into a file - TODO: replace with a proper request parser
        if (std::ofstream requestFile("request_" + std::to_string(clientFd) + ".txt");
            requestFile.is_open())
        {
            requestFile << clientRequests[clientFd];
            requestFile.close();
        }
        else
        {
            throw std::runtime_error("Error opening file to write request: " + std::string(strerror(errno)));
        }
    }
    else if (bytesRead == 0)
    {
        // Client closed connection
        std::cout << "Client " << clientFd << " closed connection" << std::endl;
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
            std::cout << "Sent response to client " << clientFd << std::endl;
        }
        else
        {
            throw std::runtime_error("Error sending response to client " + std::to_string(clientFd) + ": " + strerror(errno));
        }
    }
}
