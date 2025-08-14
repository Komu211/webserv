#include "CGISubprocess.hpp"

CGISubprocess::CGISubprocess()
{
    if (pipe(_pipe_to_cgi) != 0)
        throw std::runtime_error("Failed to create pipe to CGI: " + std::string{strerror(errno)});
    setNonBlocking(_pipe_to_cgi[0]);
    setNonBlocking(_pipe_to_cgi[1]);
    if (pipe(_pipe_from_cgi) != 0)
    {
        close(_pipe_to_cgi[0]);
        _pipe_to_cgi[0] = -1;
        close(_pipe_to_cgi[1]);
        _pipe_to_cgi[1] = -1;
        throw std::runtime_error("Failed to create pipe from CGI: " + std::string{strerror(errno)});
    }
    setNonBlocking(_pipe_from_cgi[0]);
    setNonBlocking(_pipe_from_cgi[1]);
}

CGISubprocess::~CGISubprocess()
{
    if (_pipe_to_cgi[0] != -1)
        close(_pipe_to_cgi[0]);
    if (_pipe_to_cgi[1] != -1)
        close(_pipe_to_cgi[1]);
    if (_pipe_from_cgi[0] != -1)
        close(_pipe_from_cgi[0]);
    if (_pipe_from_cgi[1] != -1)
        close(_pipe_from_cgi[1]);
    killSubprocess();
}

// void CGISubprocess::setNonBlocking(int fd)
// {
//     int flags = fcntl(fd, F_GETFL, 0); // ? is this flag allowed
//     if (flags == -1)
//     {
//         close(fd);
//         throw std::runtime_error("Failed to get pipe fd flags: " + std::string{strerror(errno)});
//     }

//     if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
//     {
//         close(fd);
//         throw std::runtime_error("Failed to set pipe fd to non-blocking mode: " + std::string{strerror(errno)});
//     }
// }

void CGISubprocess::setEnvironment(const std::unordered_map<std::string, std::string> &envMap)
{
    _envp.reserve(envMap.size() + 1);
    _envStorage.reserve(envMap.size());
    for (const auto &[key, value] : envMap)
    {
        std::string curVarPair{key + "=" + value};
        // Save each character in a vector of characters
        _envStorage.emplace_back(curVarPair.begin(), curVarPair.end());
        _envStorage.back().push_back('\0');
        _envp.push_back(_envStorage.back().data());
    }
    _envp.push_back(nullptr);
}

void CGISubprocess::createSubprocess(const std::filesystem::path &filePathAbs, const std::string &interpreter)
{
    // fork
    auto _pid = fork();
    if (_pid == -1)
        throw std::runtime_error("Failed to create fork for CGI: " + std::string{strerror(errno)});

    // in child
    else if (_pid == 0)
    {
        // change current working directory to script directory
        std::filesystem::current_path(filePathAbs.parent_path());

        redirectPipesChild();

        // prepare args for execve
        char *args[] = {const_cast<char *>(interpreter.c_str()), const_cast<char *>(filePathAbs.c_str()), NULL};

        // execve
        if (execve(args[0], args, _envp.data()) == -1)
        {
            // can also simply use std::exit but it won't clean up the local objects (destructor will not be called)
            g_shutdownServer = 2;
            throw std::runtime_error("execve failed: " + std::string{strerror(errno)});
        }
    }
    // in parent
    else if (_pid > 0)
    {
        _subprocessStarted = true;
        // close unneeded pipes
        close(_pipe_to_cgi[0]);
        _pipe_to_cgi[0] = -1;
        close(_pipe_from_cgi[1]);
        _pipe_from_cgi[1] = -1;
    }
}

void CGISubprocess::redirectPipesChild()
{
    close(_pipe_to_cgi[1]); // Close write end of the pipe (parent will write to it)
    _pipe_to_cgi[1] = -1;
    // redirect stdin to read end of pipe_to_cgi
    if (dup2(_pipe_to_cgi[0], STDIN_FILENO) == -1)
    {
        g_shutdownServer = 2;
        throw std::runtime_error("dup2 failed: " + std::string{strerror(errno)});
    }
    close(_pipe_to_cgi[0]); // close redirected fd
    _pipe_to_cgi[0] = -1;

    close(_pipe_from_cgi[0]); // close read end of the pipe (parent will read from it)
    _pipe_from_cgi[0] = -1;
    // redirect stdout to write end of pipe_from_cgi
    if (dup2(_pipe_from_cgi[1], STDOUT_FILENO) == -1)
    {
        // Maybe need a better way to exit since this copy of the server might process and
        // send some responses on its way to shutdown
        g_shutdownServer = 2;
        throw std::runtime_error("dup2 failed: " + std::string{strerror(errno)});
    }
    close(_pipe_from_cgi[1]); // close redirected fd
    _pipe_from_cgi[1] = -1;
}

int CGISubprocess::getWritePipeToCGI()
{
    return _pipe_to_cgi[1];
}

int CGISubprocess::getReadPipeFromCGI()
{
    return _pipe_from_cgi[0];
}

bool CGISubprocess::childExitedSuccessfully()
{
    if (!_subprocessStarted)
        return false;
    waitpid(_pid, &_status, WNOHANG);
    if (WIFEXITED(_status))
        return true;
    return false;
}

int CGISubprocess::getChildExitStatus()
{
    if (!_subprocessStarted)
        return -1;
    waitpid(_pid, &_status, WNOHANG);
    if (WIFEXITED(_status))
        return WEXITSTATUS(_status);
    return -1;
}

void CGISubprocess::killSubprocess(int sig)
{
    if (_subprocessStarted)
        kill(_pid, sig);
}

// void CGISubprocess::writeToChild(const std::string &body)
// {
//     write(_pipe_to_cgi[1], body.c_str(), body.length());

//     // Close write end to signal EOF
//     close(_pipe_to_cgi[1]);
//     _pipe_to_cgi[1] = -1;
// }

// std::string CGISubprocess::readFromChild()
// {
//     char        buffer[4096];
//     ssize_t     bytes_read;
//     std::string cgi_output;
//     while ((bytes_read = read(_pipe_from_cgi[0], buffer, sizeof(buffer))) > 0)
//     {
//         cgi_output.append(buffer, bytes_read);
//     }
//     close(_pipe_from_cgi[0]);
//     _pipe_from_cgi[0] = -1;

//     return cgi_output;
// }

void CGISubprocess::waitChild()
{
    waitpid(_pid, &_status, WNOHANG);
    if (WIFEXITED(_status) && WEXITSTATUS(_status) != 0)
    {
        // CGI script exited with an error
        throw std::runtime_error("CGI script failed: " + std::string{strerror(errno)});
    }
}
