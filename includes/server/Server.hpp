#pragma once

#include "HTTPRequest.hpp"
#include "HTTPRequestFactory.hpp"
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
#include <utility>
#include <vector>

#define BUFFER_SIZE 4096

// Global volatile flag to signal shutdown
extern volatile std::sig_atomic_t g_shutdownServer;

// Forward declarations in case of circular inclusions
class GlobalConfig;
class HTTPRequest;
class HTTPRequestParser;
class PollManager;
class ServerConfig;
class HTTPRequestFactory;
class Socket;

struct PendingResponse
{
    std::string response;
    size_t      sent;
};

struct OpenFile
{
    enum ReadOrWrite
    {
        READ,
        WRITE
    };
    std::string content;
    bool        finished{false};
    bool        isCGI{false};
    ReadOrWrite fileType;
    std::size_t size{};
};

struct ClientData
{
    std::string                       partialRequest{""};
    std::unique_ptr<HTTPRequest>      parsedRequest{nullptr};
    PendingResponse                   pendingResponse;
    const ServerConfig               *serverConfig;
    std::unordered_map<int, OpenFile> openFiles;
    std::string                       hostName;
    std::string                       port;
};

class Server
{
private:
    GlobalConfig                                     _global_config;
    std::unordered_map<int, std::unique_ptr<Socket>> _sockets;

    std::unordered_map<int, const ServerConfig *> _socket_to_server_config;

    PollManager                         _pollManager;
    std::unordered_map<int, ClientData> _clientData;
    std::unordered_set<int>             _clientsToRemove;
    std::unordered_set<int>             _filesToRemove;
    std::unordered_map<int, int>        _openFilesToClientMap;

    void            acceptNewConnections();
    void            acceptNewConnection(int serverFd);
    void            readFromOpenFiles();
    void            writeToOpenFiles();
    ClientData     &getClientOfFile(int fileFd);
    void            readFromClients();
    std::string     readFromClientOrFile(int fd, std::string partialContent);
    void            writeToFile(int fileFd, ClientData &client_data);
    void            respondToClients();
    void            respondToClient(int clientFd);
    void            closeConnections();
    void            closeDoneFiles();
    void            closeClientFiles(int fd);
    PendingResponse writeResponseToClient(int clientFd);

    const LocationConfig *findLocationConfig(const std::string &uri, const ServerConfig *server_config) const;

public: // used by HTTPRequest
    std::unordered_map<int, ClientData> &getClientDataMap();
    std::unordered_map<int, int>        &getOpenFilesToClientMap();
    PollManager                         &getPollManager();

public:
    Server() = delete;
    explicit Server(std::string configFileName);
    Server(const Server &src) = delete;
    Server(Server &&src) = delete;
    Server &operator=(const Server &src) = delete;
    Server &operator=(Server &&src) = delete;
    ~Server() = default;

    void fillPollManager();
    void run();
};
