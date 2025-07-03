#include "Server.hpp"

#include <fstream>

#include <fstream>

Server::Server(std::vector<ServerConfig> configs) : _configs(std::move(configs))
{
    // Create listening sockets
    for (const auto &server_config : _global_config.getServerConfigs())
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
        for (int serverFd : _pollManager.getReadableServerSockets())
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
                std::cerr << "Accept error: " << strerror(errno) << std::endl;
            }
        }

        // Stage 2: Read from clients
        std::vector<int> clientsToRemove;
        for (int clientFd : _pollManager.getReadableClientSockets())
        {
            char    buffer[1024];
            ssize_t bytesRead = read(clientFd, buffer, sizeof(buffer) - 1);

            if (bytesRead > 0)
            {
                buffer[bytesRead] = '\0';
                clientRequests[clientFd] += buffer;

                // Put the recieved request into a file
                if (std::ofstream requestFile("request_" + std::to_string(clientFd) + ".txt");
                    requestFile.is_open())
                {
                    requestFile << clientRequests[clientFd];
                    requestFile.close();
                }
                else
                {
                    std::cerr << "Error opening file for writing: " << strerror(errno) << std::endl;
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
                // Error reading
                std::cerr << "Error reading from client " << clientFd << ": " << strerror(errno) << std::endl;
                clientsToRemove.push_back(clientFd);
            }
        }

        // Stage 3: Write responses to clients
        for (int clientFd : _pollManager.getWritableClientSockets())
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
                    clientsToRemove.push_back(clientFd);
                }
                else
                {
                    std::cerr << "Error sending response to client " << clientFd << ": " << strerror(errno) << std::endl;
                    clientsToRemove.push_back(clientFd);
                }
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
