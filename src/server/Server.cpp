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