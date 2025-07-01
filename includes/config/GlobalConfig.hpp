#pragma once

#include "ServerConfig.hpp"
#include <algorithm> /* std::transform() */
#include <cstring>   /* strerror() */
#include <fstream>   /* std::ifstream */
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept> /* std::runtime_error */
#include <string>
#include <vector>

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
    std::vector<ServerConfig> _serverConfigs{};

    // `ServerConfig`s in string form only for use in parser
    std::vector<std::string> _serverConfigsStr{};

private:
    // Represents whether a value has already been seen in the config file (only for parser)
    bool _seen_root{false};
    bool _seen_client_max_body_size{false};
    bool _seen_autoindex{false};

public:
    // OCF
    GlobalConfig() = delete;
    GlobalConfig(const GlobalConfig &other) = default;
    GlobalConfig &operator=(const GlobalConfig &other) = default;
    GlobalConfig &operator=(GlobalConfig &&other) = default;
    ~GlobalConfig() = default;

    // Main parameterized constructor
    GlobalConfig(std::string file_name);

    // ! Getters

private:
    // Main parser
    void parseConfFile(std::ifstream &file_stream);
    // Helper used by parser
    void setConfigurationValue(std::string currentDirective);

    // Setters don't need to be public
    void setRoot(std::string directive);
    void setClientMaxBodySize(std::string directive);
    void setAutoIndex(std::string directive);
    void setErrorPage(std::string directive);
    void setIndex(std::string directive);
};

// Helper functions
std::string              iFStreamToString(std::ifstream &file_stream);
void                     trim(std::string &str, const std::string &charset = " \t\n\r\f\v");
std::vector<std::string> splitStr(const std::string &str, const std::string &charset = " \t\n\r\f\v");