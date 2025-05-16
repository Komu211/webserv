#pragma once

#include <iostream>
#include <string>
#include <unistd.h>

class Socket
{
private:
    std::string _host;
    int _port;
    int _fd;

public:
    Socket (std::string host, int port);
    Socket (const Socket &src) = default;
    Socket &operator= (const Socket &src) = default;
    ~Socket ();

    bool operator== (const Socket &other) const;
    bool operator!= (const Socket &other) const;

    [[nodiscard]] std::string get_host() const;
    [[nodiscard]] int get_port() const;
    [[nodiscard]] int get_fd() const;

    void set_host(const std::string &host);
    void set_port(int port);
    void set_fd(int fd);
};

std::ostream &operator<<(std::ostream &os, const Socket &socket);