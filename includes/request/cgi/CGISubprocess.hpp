#pragma once

#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include "utils.hpp"

// Global volatile flag to signal shutdown
extern volatile std::sig_atomic_t g_shutdownServer;

class CGISubprocess
{
public:
    CGISubprocess();
    CGISubprocess(const CGISubprocess &other) = delete;
    CGISubprocess &operator=(const CGISubprocess &other) = delete;
    ~CGISubprocess();

    void        setEnvironment(const std::unordered_map<std::string, std::string> &envMap);
    void        createSubprocess(const std::filesystem::path &filePathAbs, const std::string &interpreter);
    int getWritePipeToCGI();
    int getReadPipeFromCGI();
    bool childExitedSuccessfully();
    int getChildExitStatus();
    void killSubprocess(int sig=SIGTERM);

    // void        writeToChild(const std::string &body);
    // std::string readFromChild();
    void        waitChild();

private:
    int   _pipe_to_cgi[2]{-1, -1};
    int   _pipe_from_cgi[2]{-1, -1};
    pid_t _pid;
    int _status;
    bool _subprocessStarted{false};

    // Data to be passed to execve
    std::vector<char *> _envp;

    // Helper vector that is the owner of all the `char *` stored in _envp
    std::vector<std::vector<char>> _envStorage;

private:
    // redirect relevant pipe ends to stdin and stdout, and close the rest
    void redirectPipesChild();
    // void setNonBlocking(int fd);
};
