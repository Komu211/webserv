#include "Server.hpp"
#include <iostream>
#include <string>

int main()
{
    std::cout << "Hello from the wondrous webserv!" << std::endl;
    ServerConfig config;
    Server       server({config});
    server.fillActiveSockets();
    server.run();
    return (0);
}