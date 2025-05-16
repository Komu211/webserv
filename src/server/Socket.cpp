#include "Socket.hpp"

Socket::Socket(std::string host, int port) : _host(std::move(host)), _port(port), _fd(-1)
{}

Socket::~Socket()
{
    if (_fd != -1)
    {
        close(_fd);
    }
}

bool Socket::operator==(const Socket &other) const
{
    return _host == other._host && _port == other._port;
}

bool Socket::operator!=(const Socket &other) const
{
    return !(*this == other);
}

std::string Socket::get_host() const
{
    return _host;
}

int Socket::get_port() const
{
    return _port;
}

int Socket::get_fd() const
{
    return _fd;
}

void Socket::set_host(const std::string &host)
{
    _host = host;
}

void Socket::set_port(int port)
{
    _port = port;
}

void Socket::set_fd(int fd)
{
    _fd = fd;
}

std::ostream &operator<<(std::ostream &os, const Socket &socket)
{
    os << "Socket(host: " << socket.get_host() << ", port: " << socket.get_port() << ", fd: " << socket.get_fd() << ")";
    return os;
}