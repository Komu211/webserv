#pragma once

#include "ActiveSockets.hpp"
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

class Server
{
private:
    std::vector<ServerConfig>                   _configs;
    std::unordered_set<std::unique_ptr<Socket>> _sockets;
    ActiveSockets                               _activeSockets;

public:
    explicit Server(std::vector<ServerConfig> configs);
    Server(const Server &src) = default;
    Server(Server &&src) = default;
    Server &operator=(const Server &src) = default;
    Server &operator=(Server &&src) = default;
    ~Server() = default;

    // Getters and Setters
    [[nodiscard]] std::vector<ServerConfig> get_configs() const;
    void                                    set_configs(const std::vector<ServerConfig> &configs);

    void fillActiveSockets();
    void run();
};