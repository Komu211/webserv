#pragma once

#include "ActiveSockets.hpp"
#include "GlobalConfig.hpp"
#include "ServerConfig.hpp"
#include "Socket.hpp"
#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <poll.h>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

// Global volatile flag to signal shutdown
extern volatile sig_atomic_t g_shutdownServer;

class GlobalConfig;

class Server
{
private:
    GlobalConfig                                _global_config;
    std::vector<ServerConfig>                   _configs; // ! remove
    std::unordered_set<std::unique_ptr<Socket>> _sockets;
    ActiveSockets                               _activeSockets;

public:
    explicit Server(std::string configFileName);
    // explicit Server(std::vector<ServerConfig> configs); // ! remove
    Server(const Server &src) = delete; // Cannot copy (has const members) and no need to copy
    Server(Server &&src) = default;
    Server &operator=(const Server &src) = delete; // Cannot copy (has const members) and no need to copy
    Server &operator=(Server &&src) = delete;      // Cannot copy/move (has const members) and no need to copy/move
    ~Server() = default;

    // Getters and Setters
    [[nodiscard]] std::vector<ServerConfig> get_configs() const;                                   // ! remove
    void                                    set_configs(const std::vector<ServerConfig> &configs); // ! remove

    void fillActiveSockets();
    void run();
};