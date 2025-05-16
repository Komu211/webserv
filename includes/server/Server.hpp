#pragma once

#include "ServerConfig.hpp"
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

class Server
{
private:
    std::vector<ServerConfig> _configs;

public:
    Server(std::vector<ServerConfig> configs);
    Server(const Server &src) = default;
    Server(Server &&src) = default;
    Server &operator=(const Server &src) = default;
    Server &operator=(Server &&src) = default;
    ~Server() = default;

    void run();
};