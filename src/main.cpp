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
        // GlobalConfig globalConfig{GlobalConfig(argv[1])}; // ! remove
        std::cout << "Hello from the wondrous webserv!" << '\n';
        // ServerConfig config1; // ! remove
        // ServerConfig config2; // ! remove
        // ServerConfig config3; // ! remove

    config2.setPort(8081);
    config3.setPort(8082);

    config3.setHost("0.0.0.0");

    Server server({config1, config2, config3});
    server.fillPollManager();
    server.run();
    return (0);
}