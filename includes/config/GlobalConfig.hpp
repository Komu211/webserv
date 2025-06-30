#pragma once

#include "ServerConfig.hpp"
#include <cstring> /* strerror() */
#include <fstream> /* std::ifstream */
#include <map>
#include <stdexcept> /* std::runtime_error */
#include <string>
#include <vector>
#include <iostream>

class ServerConfig;

class GlobalConfig
{
private:
    // Root directory for requests
    std::string _root{"./html"};

    // Files that will be used as an index (checked in this order)
    std::vector<std::string> _index_files_vec{};

    // The maximum allowed size (in bytes) of the client request body
    std::size_t _client_max_body_size{1000000};

    // Enables or disables the directory listing output
    bool _autoindex{false};

    // URI that will be shown for the specified error codes (must be between 300 and 599)
    std::map<int, std::string> _error_pages_map{};

    // `ServerConfig`s
    std::vector<ServerConfig> _serverConfigSet;

public:
    // OCF
    GlobalConfig() = delete;
    GlobalConfig(const GlobalConfig &other) = default;
    GlobalConfig &operator=(const GlobalConfig &other) = default;
    GlobalConfig &operator=(GlobalConfig &&other) = default;
    ~GlobalConfig() = default;

    // Main parameterized constructor
    GlobalConfig(std::string file_name);

    // Main parser
    void parseConfFile(std::ifstream &file_stream);

    // ! Getters

    // No setters needed
};

// Helper functions
void trimWhitespace(std::string &str, const std::string &whitespace = " \t\n\r\f\v");
