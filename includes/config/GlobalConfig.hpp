#pragma once

#include "ServerConfig.hpp"
#include "utils.hpp"
#include <algorithm> /* std::transform() */
#include <cstring>   /* strerror() */
#include <fstream>   /* std::ifstream */
#include <iostream>
#include <limits>
#include <map>
#include <memory>    /* std::unique_ptr */
#include <stdexcept> /* std::runtime_error */
#include <string>
#include <vector>

class ServerConfig;

class GlobalConfig
{
public:
    // Main parameterized constructor
    GlobalConfig(std::string file_name);

    // OCF
    GlobalConfig() = delete;
    GlobalConfig(const GlobalConfig &other) = delete;
    GlobalConfig(GlobalConfig &&other) = default;
    GlobalConfig &operator=(const GlobalConfig &other) = delete;
    GlobalConfig &operator=(GlobalConfig &&other) = default;
    ~GlobalConfig() = default;

    // Getters
    const std::string                                &getRoot() const;
    const std::vector<std::string>                   &getIndexFiles() const;
    std::size_t                                       getClientMaxBodySize() const;
    bool                                              getAutoIndex() const;
    const std::map<int, std::string>                 &getErrorPagesMap() const;
    const std::vector<std::unique_ptr<ServerConfig>> &getServerConfigs() const;

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
    std::vector<std::unique_ptr<ServerConfig>> _serverConfigs{};

private: // Data members for parser only
    // Represents whether a value has already been seen in the config file
    bool _seen_root{false};
    bool _seen_client_max_body_size{false};
    bool _seen_autoindex{false};
    bool _seen_index{false};

    // `ServerConfig`s in string form only for use in parser
    std::vector<std::string> _serverConfigsStr{};

private: // Member functions for parser only
    // Main parser
    void parseConfFile(std::ifstream &file_stream);
    // Helper used by parser
    void setConfigurationValue(std::string directive);

    // Setters don't need to be public
    void setRoot(std::string directive);
    void setClientMaxBodySize(std::string directive);
    void setAutoIndex(std::string directive);
    void setErrorPage(std::string directive);
    void setIndex(std::string directive);
};