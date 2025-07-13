#pragma once

#include <poll.h>
#include <unordered_map>
#include <vector>

class PollManager
{
private:
    std::vector<pollfd>    _pollfds;
    std::unordered_map<int, bool> _isServerSocket;

public:
    PollManager();
    pollfd              *data();
    [[nodiscard]] std::size_t size() const;
    void                 addSocket(int fd, short events);
    void                 addServerSocket(int fd);
    void                 addClientSocket(int fd);
    void                 removeSocket(int fd);
    void                 setEvents(int fd, short events);
    void                 updateEvents(int fd, short events);
    void                 removeEvents(int fd, short events);

    [[nodiscard]] bool isReadable(int fd) const;
    [[nodiscard]] bool isWritable(int fd) const;
    [[nodiscard]] bool isServerSocket(int fd) const;

    [[nodiscard]] std::vector<int> getReadableServerSockets() const;
    [[nodiscard]] std::vector<int> getReadableClientSockets() const;
    [[nodiscard]] std::vector<int> getWritableClientSockets() const;

    std::vector<pollfd> getPollFDs();
};
