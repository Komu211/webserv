#pragma once

#include <arpa/inet.h>
#include <cerrno>
#include <cstring> /* strerror() */
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <netdb.h> /* struct addrinfo */
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h> /* socket() */
#include <unistd.h>
#include <fstream>

// Pair of strings (used for host:port combinations)
using StringPair = std::pair<std::string, std::string>;
// Pair of `struct addrinfo` and human-readable host:port strings
using AddrInfoPair = std::pair<struct addrinfo, StringPair>;

class Socket
{
public:
    explicit Socket(const AddrInfoPair &addr_info_pair);
    Socket(const Socket &src) = delete;            // Cannot copy (has const members) and no need to copy
    Socket &operator=(const Socket &src) = delete; // Cannot copy (has const members) and no need to copy
    ~Socket();

    bool operator==(const Socket &other) const;
    bool operator!=(const Socket &other) const;

    [[nodiscard]] const std::string& get_host() const;
    [[nodiscard]] const std::string& get_port() const;
    [[nodiscard]] int         get_fd() const;

    // void set_host(const std::string &host); // Not needed
    // void set_port(int port);                // Not needed
    // void set_fd(int fd);                    // Not needed (can leak file descriptors if used)

    void initSocket();

private:
    const struct addrinfo &_addr_info_struct;
    std::string _host;
    std::string _port;
    int         _fd;

    void createSocket();
    void setNonBlocking();
    void bindSocket();
    void listenSocket(int backlog = SOMAXCONN);
};

std::ostream &operator<<(std::ostream &os, const Socket &socket);
