#include "GlobalConfig.hpp"
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

int main(int argc, char **argv)
{
    // Register signal handler for graceful shutdown
    if (signal(SIGINT, signal_handler) == SIG_ERR)
    {
        std::cerr << "Failed to register SIGINT handler." << '\n';
        return 1;
    }

    if (argc != 2)
    {
        std::cout << "Usage: ./webserv <configuration file>" << '\n';
        return 1;
    }

    try
    {
        std::cout << "Hello from the wondrous webserv!" << '\n';

        Server server{argv[1]};
        server.fillPollManager();
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return (0);
}