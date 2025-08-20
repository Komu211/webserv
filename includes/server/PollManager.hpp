#pragma once

#include <poll.h>
#include <unordered_map>
#include <vector>

class PollManager
{
private:
    enum SocketType
    {
        SERVER,
        CLIENT,
        READFILE,
        WRITEFILE
    };

    std::vector<pollfd>                 _pollfds;
    std::unordered_map<int, SocketType> _sockTypeMap;
    // std::unordered_map<int, bool> _isServerSocket;
    // std::unordered_map<int, bool> _isReadFile;
    // std::unordered_map<int, bool> _isWriteFile;

public:
    PollManager() = default;
    PollManager(const PollManager &other) = delete;
    PollManager &operator=(const PollManager &other) = delete;
    ~PollManager() = default;

    pollfd                   *data();
    [[nodiscard]] std::size_t size() const;
    void                      addSocket(int fd, short events);
    void                      addServerSocket(int fd);
    void                      addClientSocket(int fd);
    void                      addReadFileFd(int fd);
    void                      addWriteFileFd(int fd);
    void                      removeSocket(int fd);
    void                      setEvents(int fd, short events);
    void                      updateEvents(int fd, short events);
    void                      removeEvents(int fd, short events);

    [[nodiscard]] bool isReadable(int fd) const;
    [[nodiscard]] bool isWritable(int fd) const;
    [[nodiscard]] bool isServerSocket(int fd) const;
    [[nodiscard]] bool isClientSocket(int fd) const;
    [[nodiscard]] bool isReadFileSocket(int fd) const;
    [[nodiscard]] bool isWriteFileSocket(int fd) const;

    [[nodiscard]] std::vector<int> getReadableServerSockets() const;
    [[nodiscard]] std::vector<int> getReadableClientSockets() const;
    [[nodiscard]] std::vector<int> getWritableClientSockets() const;
    [[nodiscard]] std::vector<int> getWritableFiles() const;
    [[nodiscard]] std::vector<int> getReadableFiles() const;

    std::vector<pollfd> getPollFDs();
};
