#pragma once

#define DEFAULT_CONFIGURATION_FILE "config.json"

#include <string>
#include <iostream>

class Server
{
private:
	std::string _name;

public:
	Server(std::string configFile = DEFAULT_CONFIGURATION_FILE);
	Server(const Server &src) = default;
	Server(Server &&src) = default;
	Server &operator=(const Server &src) = default;
	Server &operator=(Server &&src) = default;
	~Server() = default;

	void run();
};