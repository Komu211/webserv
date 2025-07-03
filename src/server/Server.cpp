#include "Server.hpp"

Server::Server(std::string configFileName)
    : _global_config{configFileName} // Initiate parsing of the config file
{
    // Create listening sockets
    for (const auto &server_config : _global_config.getServerConfigs())
    {
        // Each server_config can be listening on multiple host_port combinations
        for (auto &addr_info_pair : server_config->getAddrInfoVec())
        {
            auto newSocket = std::make_unique<Socket>(addr_info_pair);
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
                std::cerr << "Error initializing socket: " << e.what() << '\n';
                continue;
            }
            _sockets.insert(std::move(newSocket));
        }
    }
}

// ! remove
// Server::Server(std::vector<ServerConfig> configs)
//     : _configs(std::move(configs))
// {
//     for (const auto &config : _configs)
//     {
//         auto newSocket = std::make_unique<Socket>(config.getHost(), config.getPort());
//         bool exists = false;
//         for (const auto &existingSocket : _sockets)
//         {
//             if (*existingSocket == *newSocket)
//                 exists = true;
//         }
//         if (exists)
//             continue;
//         try
//         {
//             newSocket->initSocket();
//         }
//         catch (const std::runtime_error &e)
//         {
//             std::cerr << "Error initializing socket: " << e.what() << '\n';
//             continue;
//         }
//         _sockets.insert(std::move(newSocket));
//     }
// }

// * Getter not needed
// * Inefficient: making a copy of vector on every return (use const std::vector<>&)
// std::vector<ServerConfig> Server::get_configs() const
// {
//     return _configs;
// }

// * Setter not needed
// void Server::set_configs(const std::vector<ServerConfig> &configs)
// {
//     _configs = configs;
// }

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
    std::cout << "Server is started successfully. Listening on " << _activeSockets.size() << " addresses..." << '\n';
    // TODO: Add printing of host:port of sockets currently being listened to

    // * check if server starts with listening to 0 connections

    while (g_shutdownServer == 0)
    {
        /*
         *TODO:
         * 1. Poll for events on the watched FDs ✅
         * 2. Accept new connections ✅
         * 3. Read requests from clients
         * 4. Read available internal files
         * 5. Send responses to clients
         * 6. Close finished and timeout connections
         */

        // std::cout << "Waiting for requests on sockets: " << '\n';
        // for (const auto &sockPtr : _sockets)
        // {
        //     std::cout << *sockPtr << '\n';
        // }
        const int pollResult = poll(_activeSockets.data(), _activeSockets.size(), -1);
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
        for (auto &pollFd : _activeSockets.getPollFDs())
        {
            if (pollFd.revents & POLLIN)
            {
                int clientFd = accept(pollFd.fd, nullptr, nullptr);
                if (clientFd >= 0)
                {
                    std::cout << "Accepted new connection: " << clientFd << '\n';
                    char    buffer[1024];
                    ssize_t bytesRead = read(clientFd, buffer, sizeof(buffer) - 1);
                    if (bytesRead > 0)
                    {
                        buffer[bytesRead] = '\0';
                        std::cout << "Received request:\n" << buffer << '\n';
                    }

                    std::string response = "HTTP/1.1 200 OK\r\n"
                                           "Content-Type: text/plain\r\n"
                                           "Content-Length: 13\r\n"
                                           "\r\n"
                                           "Hello, world!";
                    send(clientFd, response.c_str(), response.size(), 0);
                    // close(clientFd);
                }
                else
                {
                    std::cerr << "Accept error: " << strerror(errno) << '\n';
                }
            }
        }
    }

    std::cout << "Server successfully stopped. Goodbye!" << '\n';
}