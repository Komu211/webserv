#include "ActiveSockets.hpp"

ActiveSockets::ActiveSockets()
{}

pollfd *ActiveSockets::data()
{
    return _pollfds.data();
}

size_t ActiveSockets::size() const
{
    return _pollfds.size();
}

void ActiveSockets::addSocket(int fd, short events)
{
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = events;
    _pollfds.push_back(pfd);
}

void ActiveSockets::removeSocket(int fd)
{
    for (auto it = _pollfds.begin(); it != _pollfds.end(); ++it)
    {
        if (it->fd == fd)
        {
            _pollfds.erase(it);
            break;
        }
    }
}

std::vector<struct pollfd> ActiveSockets::getPollFDs()
{
    return _pollfds;
}