#pragma once

#include <poll.h>
#include <unordered_set>
#include <vector>

class ActiveSockets
{
private:
    std::vector<struct pollfd> _pollfds;

public:
    ActiveSockets();
    pollfd                    *data();
    [[nodiscard]] std::size_t  size() const;
    void                       addSocket(int fd, short events);
    void                       removeSocket(int fd);
    std::vector<struct pollfd> getPollFDs();
};
