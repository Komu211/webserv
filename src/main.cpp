#include "Server.hpp"
#include <csignal> /* signal() */
#include <iostream>
#include <string>

// Global volatile flag to signal shutdown
volatile sig_atomic_t g_shutdownServer = 0;

// Signal handler function
void signal_handler(int signum)
{
    if (signum == SIGINT)
    {
        std::cout << "\nSIGINT received. Initiating server shutdown..." << '\n';
        g_shutdownServer = 1;
    }
}

int main()
{
    // Register signal handler for graceful shutdown
    if (signal(SIGINT, signal_handler) == SIG_ERR)
    {
        std::cerr << "Failed to register SIGINT handler." << '\n';
        return 1;
    }

    std::cout << "Hello from the wondrous webserv!" << std::endl;
    ServerConfig config1;
    ServerConfig config2;
    ServerConfig config3;

    config2.setPort(8081);
    config3.setPort(8082);

    Server server({config1, config2, config3});
    server.fillActiveSockets();
    server.run();
    return (0);
}