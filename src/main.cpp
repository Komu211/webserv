#include <iostream>
#include <string>
#include "Server.hpp"

int main()
{
	std::cout << "Hello from the wondrous webserv!" << std::endl;
	Server server;
	server.run();
	return (0);
}