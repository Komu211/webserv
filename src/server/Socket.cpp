#include "Socket.hpp"

Socket::Socket(const AddrInfoPair &addr_info_pair)
    : _addr_info_struct{addr_info_pair.first}
    , _host{addr_info_pair.second.first}
    , _port{addr_info_pair.second.second}
    , _fd{-1}
{
}

// Socket::Socket(std::string host, int port)
//     : _host(std::move(host))
//     , _port(port)
//     , _fd(-1)
// {
// }

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

std::string Socket::get_port() const
{
    return _port;
}

// int Socket::get_port() const
// {
//     return _port;
// }

int Socket::get_fd() const
{
    return _fd;
}

// void Socket::set_host(const std::string &host)
// {
//     _host = host;
// }

// void Socket::set_port(int port)
// {
//     _port = port;
// }

// void Socket::set_fd(int fd)
// {
//     _fd = fd;
// }

void Socket::createSocket()
{
    _fd = socket(_addr_info_struct.ai_family, _addr_info_struct.ai_socktype, _addr_info_struct.ai_protocol);
    if (_fd < 0)
    {
        throw std::runtime_error("Failed to create socket: " + std::string{strerror(errno)});
    }

    // Set socket option to reuse address
    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        close(_fd);
        throw std::runtime_error("Failed to set socket options: " + std::string{strerror(errno)});
    }
}

void Socket::setNonBlocking()
{
    int flags = fcntl(_fd, F_GETFL, 0); // ? is this flag allowed
    if (flags == -1)
    {
        close(_fd);
        throw std::runtime_error("Failed to get socket flags: " + std::string{strerror(errno)});
    }

    if (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        close(_fd);
        throw std::runtime_error("Failed to set socket to non-blocking mode: " + std::string{strerror(errno)});
    }
}

void Socket::bindSocket()
{
    // struct sockaddr_in address;
    // std::memset(&address, 0, sizeof(address));

    // address.sin_family = AF_INET;
    // address.sin_port = htons(_port);

    // // Convert host string to network address
    // if (_host == "localhost" || _host == "127.0.0.1")
    // {
    //     address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    // }
    // else if (_host == "0.0.0.0" || _host.empty())
    // {
    //     address.sin_addr.s_addr = htonl(INADDR_ANY);
    // }
    // else
    // {
    //     if (inet_pton(AF_INET, _host.c_str(), &address.sin_addr) <= 0) // ! forbidden function
    //     {
    //         close(_fd);
    //         throw std::runtime_error("Invalid address: " + _host);
    //     }
    // }

    if (bind(_fd, _addr_info_struct.ai_addr, _addr_info_struct.ai_addrlen) < 0)
    {
        close(_fd);
        throw std::runtime_error("Failed to bind socket to `" + _host + ":" + _port + "`: " + strerror(errno));
    }
}

void Socket::listenSocket(int backlog)
{
    if (listen(_fd, backlog) < 0)
    {
        close(_fd);
        throw std::runtime_error("Failed to listen on socket: " + std::string{strerror(errno)});
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
        std::cout << "Socket initialized successfully on " << _host << ":" << _port << '\n';
    }
    catch (const std::exception &e)
    {
        // std::cerr << "Socket initialization error: " << e.what() << '\n';
        if (_fd != -1)
        {
            close(_fd);
            _fd = -1;
        }
        throw std::runtime_error(std::string{e.what()});
    }
}

std::ostream &operator<<(std::ostream &os, const Socket &socket)
{
    std::string url{socket.get_host() + ":" + socket.get_port()};
    if (url.substr(0, 7) != "http://")
        url.insert(0, "http://");

    os << "Socket( host: " << socket.get_host() << ", port: " << socket.get_port() << ", fd: " << socket.get_fd() << ", url: " << url << " )";
    return os;
}
