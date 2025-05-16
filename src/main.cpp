#include <iostream>
#include <string>
#include "Server.hpp"

int main()
{
	std::cout << "Hello from the wondrous webserv!" << std::endl;
	ServerConfig config;
	Server server({config});
	server.run();
	return (0);
}