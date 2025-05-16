#include "Server.hpp"

Server::Server(std::vector<ServerConfig> configs) : _configs(std::move(configs))
{
    for (const auto &config : _configs)
    {
        auto newSocket = std::make_unique<Socket>(config.getHost(), config.getPort());
        bool exists = false;
        for (const auto &existingSocket : _sockets)
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
        _sockets.insert(std::move(newSocket));
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

void Server::fillActiveSockets()
{
    for (const auto &sockPtr : _sockets)
    {
        int fd = sockPtr->get_fd();
        _activeSockets.addSocket(fd, POLLIN);
    }
}

void Server::run()
{
    std::cout << "Server is running..." << std::endl;
    while (42)
    {
        /*
         *TODO:
         * 1. Poll for events on the watched FDs
         * 2. Accept new connections
         * 3. Read requests from clients
         * 4. Read available internal files
         * 5. Send responses to clients
         * 6. Close finished and timeout connections
         */

        // std::cout << "Waiting for requests on sockets: " << std::endl;
        // for (const auto &sockPtr : _sockets)
        // {
        //     std::cout << *sockPtr << std::endl;
        // }
        const int pollResult = poll(_activeSockets.data(), _activeSockets.size(), 2000);
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
        for (auto &pollFd : _activeSockets.getPollFDs())
        {
            if (pollFd.revents & POLLIN)
            {
                if (pollFd.fd == 3)
                {
                    int clientFd = accept(pollFd.fd, nullptr, nullptr);
                    if (clientFd >= 0)
                    {
                        std::cout << "Accepted new connection: " << clientFd << std::endl;
                        char buffer[1024];
                        ssize_t bytesRead = read(clientFd, buffer, sizeof(buffer) - 1);
                        if (bytesRead > 0)
                        {
                            buffer[bytesRead] = '\0';
                            std::cout << "Received request:\n"
                                      << buffer << std::endl;
                        }

                        std::string response = "HTTP/1.1 200 OK\r\n"
                                               "Content-Type: text/plain\r\n"
                                               "Content-Length: 13\r\n"
                                               "\r\n"
                                               "Hello, world!";
                        send(clientFd, response.c_str(), response.size(), 0);
                        close(clientFd);
                    }
                }

                else
                {
                    std::cout << "Client socket ready to read: " << pollFd.fd << std::endl;
                }
            }
        }

        // Sleep for 10 seconds
    }
}