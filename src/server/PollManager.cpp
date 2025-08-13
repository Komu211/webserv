#include "PollManager.hpp"

pollfd *PollManager::data()
{
    return _pollfds.data();
}

std::size_t PollManager::size() const
{
    return _pollfds.size();
}

void PollManager::addSocket(const int fd, const short events)
{
    pollfd pfd{};
    pfd.fd = fd;
    pfd.events = events;
    pfd.revents = 0;
    _pollfds.push_back(pfd);
}

void PollManager::addServerSocket(int fd)
{
    addSocket(fd, POLLIN);
    _sockTypeMap[fd] = SERVER;
    // _isServerSocket[fd] = true;
}

void PollManager::addClientSocket(int fd)
{
    addSocket(fd, POLLIN);
    _sockTypeMap[fd] = CLIENT;
    // _isServerSocket[fd] = false;
}

void PollManager::addReadFileFd(int fd)
{
    addSocket(fd, POLLIN);
    _sockTypeMap[fd] = READFILE;
    // _isServerSocket[fd] = false;
}

void PollManager::addWriteFileFd(int fd)
{
    addSocket(fd, POLLOUT);
    // _isServerSocket[fd] = false;
    // _isReadFile[fd] = false;
    // _isWriteFile[fd] = true;
}

void PollManager::removeSocket(int fd)
{
    for (auto it = _pollfds.begin(); it != _pollfds.end(); ++it)
    {
        if (it->fd == fd)
        {
            _pollfds.erase(it);
            _sockTypeMap.erase(fd);
            // _isServerSocket.erase(fd);
            break;
        }
    }
}

void PollManager::setEvents(int fd, short events)
{
    for (auto &pfd : _pollfds)
    {
        if (pfd.fd == fd)
        {
            pfd.events = events;
            break;
        }
    }
}

void PollManager::updateEvents(int fd, short events)
{
    for (auto &pfd : _pollfds)
    {
        if (pfd.fd == fd)
        {
            pfd.events |= events;
            break;
        }
    }
}

void PollManager::removeEvents(int fd, short events)
{
    for (auto &pfd : _pollfds)
    {
        if (pfd.fd == fd)
        {
            pfd.events &= ~events;
            break;
        }
    }
}

bool PollManager::isReadable(int fd) const
{
    for (const auto &pfd : _pollfds)
    {
        if (pfd.fd == fd && (pfd.revents & POLLIN))
            return true;
    }
    return false;
}

bool PollManager::isWritable(int fd) const
{
    for (const auto &pfd : _pollfds)
    {
        if (pfd.fd == fd && (pfd.revents & POLLOUT))
            return true;
    }
    return false;
}

bool PollManager::isServerSocket(int fd) const
{
    auto it = _sockTypeMap.find(fd);
    return it != _sockTypeMap.end() && it->second == SERVER;
    // auto it = _isServerSocket.find(fd);
    // return it != _isServerSocket.end() && it->second;
}

bool PollManager::isClientSocket(int fd) const
{
    auto it = _sockTypeMap.find(fd);
    return it != _sockTypeMap.end() && it->second == CLIENT;
}

bool PollManager::isReadFileSocket(int fd) const
{
    auto it = _sockTypeMap.find(fd);
    return it != _sockTypeMap.end() && it->second == READFILE;
}

bool PollManager::isWriteFileSocket(int fd) const
{
    auto it = _sockTypeMap.find(fd);
    return it != _sockTypeMap.end() && it->second == WRITEFILE;
}

std::vector<int> PollManager::getReadableServerSockets() const
{
    std::vector<int> readableSockets;
    for (const auto &pfd : _pollfds)
    {
        if ((pfd.revents & POLLIN) && isServerSocket(pfd.fd))
            readableSockets.push_back(pfd.fd);
    }
    return readableSockets;
}

std::vector<int> PollManager::getReadableClientSockets() const
{
    std::vector<int> readableSockets;
    for (const auto &pfd : _pollfds)
    {
        if ((pfd.revents & POLLIN) && isClientSocket(pfd.fd))
            readableSockets.push_back(pfd.fd);
    }
    return readableSockets;
}

std::vector<int> PollManager::getWritableClientSockets() const
{
    std::vector<int> writableSockets;
    for (const auto &pfd : _pollfds)
    {
        if ((pfd.revents & POLLOUT) && isClientSocket(pfd.fd))
            writableSockets.push_back(pfd.fd);
    }
    return writableSockets;
}

std::vector<int> PollManager::getReadableFiles() const
{
    std::vector<int> writableFiles;
    for (const auto &pfd : _pollfds)
    {
        if ((pfd.revents & POLLIN) && isReadFileSocket(pfd.fd))
            writableFiles.push_back(pfd.fd);
    }
    return writableFiles;
}

std::vector<int> PollManager::getWritableFiles() const
{
    std::vector<int> readableFiles;
    for (const auto &pfd : _pollfds)
    {
        if ((pfd.revents & POLLOUT) && isWriteFileSocket(pfd.fd))
            readableFiles.push_back(pfd.fd);
    }
    return readableFiles;
}

std::vector<pollfd> PollManager::getPollFDs()
{
    return _pollfds;
}
