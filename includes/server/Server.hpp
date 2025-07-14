#pragma once

#include "HTTPRequest.hpp"
#include "HTTPRequestParser.hpp"
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

#define BUFFER_SIZE 1024

// Global volatile flag to signal shutdown
extern volatile sig_atomic_t g_shutdownServer;

class GlobalConfig;

struct PendingResponse
{
    std::string response;
    size_t sent;
};

class Server
{
private:
    GlobalConfig                                     _global_config;
    std::unordered_map<int, std::unique_ptr<Socket>> _sockets;
    PollManager                                      _pollManager;

    void acceptNewConnections(int serverFd, std::unordered_map<int, std::string> &clientRequests);
    std::string readFromClient(int clientFd, std::string partialRequest);
    PendingResponse writeResponseToClient(int clientFd, std::unique_ptr<HTTPRequest> &clientRequest, PendingResponse &pendingResponse);

public:
    explicit Server(std::string configFileName);
    // explicit Server(std::vector<ServerConfig> configs); // ! remove
    Server(const Server &src) = delete; // Cannot copy GlobalConfig and no need to copy
    Server(Server &&src) = default;
    Server &operator=(const Server &src) = delete; // Cannot copy GlobalConfig and no need to copy
    Server &operator=(Server &&src) = delete;      // Cannot copy/move GlobalConfig and no need to copy/move
    ~Server() = default;

    // Getters and Setters
    // [[nodiscard]] std::vector<ServerConfig> get_configs() const;                                   // ! remove
    // void                                    set_configs(const std::vector<ServerConfig> &configs); // ! remove

    void fillPollManager();
    void run();
};
