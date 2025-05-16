#include "Server.hpp"

Server::Server(std::vector<ServerConfig> configs) : _configs(std::move(configs))
{
    try {
        for (const auto &config : _configs)
        {
            auto newSocket = Socket(config.getHost(), config.getPort());
            bool exists = false;
            for (const auto& sockPtr : _sockets) {
                if (*sockPtr == newSocket)
                {
                    exists = true;
                    break;
                }
            }
            if (exists)
                continue;
            // TODO: Initialize the socket (bind, listen, etc.)
            _sockets.insert(std::make_unique<Socket>(newSocket));
        }
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Failed to create server sockets");
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

        std::cout << "Waiting for requests on sockets: " << std::endl;
        for (const auto &sockPtr : _sockets)
        {
            std::cout << *sockPtr << std::endl;
        }
        // Sleep for 10 seconds
        std::this_thread::sleep_for(std::chrono::microseconds(10000000));
    }
}