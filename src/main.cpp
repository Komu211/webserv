#include "Server.hpp"
#include <iostream>
#include <string>

int main()
{
    std::cout << "Hello from the wondrous webserv!" << std::endl;
    ServerConfig config1;
    ServerConfig config2;
    ServerConfig config3;

    config2.setPort(8081);
    config3.setPort(8082);

    Server server({config1, config2, config3});
    server.fillActiveSockets();
    server.run();
    return (0);
}