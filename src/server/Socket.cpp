#include "Socket.hpp"

Socket::Socket(std::string host, int port)
    : _host(std::move(host))
    , _port(port)
    , _fd(-1)
{
}

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

void Socket::createSocket()
{
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0)
    {
        throw std::runtime_error("Failed to create socket");
    }

    // Set socket option to reuse address
    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        close(_fd);
        throw std::runtime_error("Failed to set socket options");
    }
}

void Socket::setNonBlocking()
{
    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags == -1)
    {
        close(_fd);
        throw std::runtime_error("Failed to get socket flags");
    }

    if (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        close(_fd);
        throw std::runtime_error("Failed to set socket to non-blocking mode");
    }
}

void Socket::bindSocket()
{
    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons(_port);

    // Convert host string to network address
    if (_host == "localhost" || _host == "127.0.0.1")
    {
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }
    else if (_host == "0.0.0.0" || _host.empty())
    {
        address.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        if (inet_pton(AF_INET, _host.c_str(), &address.sin_addr) <= 0) // ! forbidden function
        {
            close(_fd);
            throw std::runtime_error("Invalid address: " + _host);
        }
    }

    if (bind(_fd, (struct sockaddr *) &address, sizeof(address)) < 0)
    {
        close(_fd);
        throw std::runtime_error("Failed to bind socket to " + _host + ":" + std::to_string(_port));
    }
}

void Socket::listenSocket(int backlog)
{
    if (listen(_fd, backlog) < 0)
    {
        close(_fd);
        throw std::runtime_error("Failed to listen on socket");
    }
}

void Socket::initSocket()
{
    try
    {
        createSocket();
        setNonBlocking();
        bindSocket();
        listenSocket();
        std::cout << "Socket initialized successfully on " << _host << ":" << _port << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Socket initialization error: " << e.what() << std::endl;
        if (_fd != -1)
        {
            close(_fd);
            _fd = -1;
        }
        throw;
    }
}

std::ostream &operator<<(std::ostream &os, const Socket &socket)
{
    os << "Socket(host: " << socket.get_host() << ", port: " << socket.get_port() << ", fd: " << socket.get_fd() << ")";
    return os;
}
