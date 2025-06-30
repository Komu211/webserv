#pragma once

#include "PollManager.hpp"
#include "ServerConfig.hpp"
#include "Socket.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <poll.h>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class Server
{
private:
    std::vector<ServerConfig>                        _configs;
    std::unordered_map<int, std::unique_ptr<Socket>> _sockets;
    PollManager                                      _pollManager;

    void acceptNewConnections(int serverFd, std::unordered_map<int, std::string> &clientRequests);
    void readFromClient(int clientFd, std::unordered_map<int, std::string> &clientRequests, std::vector<int> &clientsToRemove);
    void writeResponseToClient(int clientFd, std::unordered_map<int, std::string> &clientRequests);

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

    void fillPollManager();
    void run();
};