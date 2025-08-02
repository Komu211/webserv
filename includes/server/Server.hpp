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
#include <unordered_set>
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

struct ClientData
{
    std::string partialRequest;
    std::unique_ptr<HTTPRequest> parsedRequest;
    PendingResponse pendingResponse;
    const ServerConfig* serverConfig;
};

class Server
{
private:
    GlobalConfig                                     _global_config;
    std::unordered_map<int, std::unique_ptr<Socket>> _sockets;
    
    std::unordered_map<int, const ServerConfig*>     _socket_to_server_config;
    
    PollManager                                      _pollManager;
    std::unordered_map<int, ClientData>              _clientData;
    std::unordered_set<int> _clientsToRemove;


    void acceptNewConnections();
    void acceptNewConnection(int serverFd);
    void readFromClients();
    std::string readFromClient(int clientFd);
    void respondToClients();
    void respondToClient(int clientFd);
    void closeConnections();
    PendingResponse writeResponseToClient(int clientFd);
    
    const LocationConfig* findLocationConfig(const std::string& uri, const ServerConfig* server_config) const;

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
