#pragma once

#include "ServerConfig.hpp"
#include "Socket.hpp"
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <stdexcept>
#include <vector>
#include <memory>
#include <unordered_set>

class Server
{
private:
    std::vector<ServerConfig> _configs;
    std::unordered_set<std::unique_ptr<Socket>> _sockets;

public:
    explicit Server(std::vector<ServerConfig> configs);
    Server(const Server &src) = default;
    Server(Server &&src) = default;
    Server &operator=(const Server &src) = default;
    Server &operator=(Server &&src) = default;
    ~Server() = default;

    // Getters and Setters
    [[nodiscard]] std::vector<ServerConfig> get_configs() const;
    void set_configs(const std::vector<ServerConfig> &configs);

    void run();
};