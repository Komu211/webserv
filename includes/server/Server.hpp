#pragma once

#include "PollManager.hpp"
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
#include <unordered_map>
#include <vector>

// Global volatile flag to signal shutdown
extern volatile sig_atomic_t g_shutdownServer;

class GlobalConfig;

class Server
{
private:
    std::vector<ServerConfig>                        _configs;
    std::unordered_map<int, std::unique_ptr<Socket>> _sockets;
    PollManager                                      _pollManager;

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

    void fillPollManager();
    void run();
};