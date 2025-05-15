#include "Server.hpp"
#include <chrono>
#include <thread>

Server::Server(std::string configFile): _name("Server name")
{
	std::cout << "Server created with config file: " << configFile << std::endl;
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