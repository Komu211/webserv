#include "Server.hpp"

Server::Server(std::vector<ServerConfig> configs) : _configs(std::move(configs))
{
    std::cout << "Server created with config files!" << std::endl;
}

void Server::run()
{
    std::cout << "Server is running..." << std::endl;

    while (42)
    {
        std::cout << "Waiting for requests..." << std::endl;

        // Sleep for 10 seconds
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}