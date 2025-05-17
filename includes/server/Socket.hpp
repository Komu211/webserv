#pragma once

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class Socket
{
private:
    std::string _host;
    int         _port;
    int         _fd;

    void createSocket();
    void setNonBlocking() const;
    void bindSocket();
    void listenSocket(int backlog = 10) const;

public:
    Socket(std::string host, int port);
    Socket(const Socket &src) = default;
    Socket &operator=(const Socket &src) = default;
    ~Socket();

    bool operator==(const Socket &other) const;
    bool operator!=(const Socket &other) const;

    [[nodiscard]] std::string get_host() const;
    [[nodiscard]] int         get_port() const;
    [[nodiscard]] int         get_fd() const;

    void set_host(const std::string &host);
    void set_port(int port);
    void set_fd(int fd);

    void initSocket();
};

std::ostream &operator<<(std::ostream &os, const Socket &socket);
