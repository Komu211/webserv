#pragma once

#include "GlobalConfig.hpp"
#include "utils.hpp"
#include <map>
#include <stdexcept>
#include <string>
#include <utility> /* std::pair */
#include <vector>

class GlobalConfig;
class LocationConfig;

class ServerConfig
{
public:
    // Main parameterized ctor (parses server_block_str, inherits the rest from global_config)
    ServerConfig(const std::string &server_block_str, const GlobalConfig &global_config);

    // OCF
    ServerConfig(); // ! Just for testing; should mark as `=delete` later
    ServerConfig(const ServerConfig &other) = default;
    ServerConfig &operator=(const ServerConfig &other) = default;
    ServerConfig &operator=(ServerConfig &&other) = default;
    ~ServerConfig();

public:
    [[nodiscard]] const std::string              &getRoot() const;
    [[nodiscard]] const std::vector<std::string> &getServerNames() const;
    // ! more getters

    // ! remove the below getters
    [[nodiscard]] std::string getHost() const; // !
    [[nodiscard]] int         getPort() const; // !

    // ! remove
    void setPort(int newPort); // !

private:
    // All `host:port` combinations this server listens to)
    std::vector<std::pair<std::string, std::string>> _listen_host_port{{"0.0.0.0", "80"}};

    // All `host:port` combinations this server listens to (in their addrinfo form)
    std::vector<struct addrinfo *> _addrinfo_lists_vec{};

    // All `server_name`s (Host header) this server responds to
    std::vector<std::string> _serverNames{};

    // Root directory for requests to this server
    std::string _root{"./html"};

    // Files that will be used as an index (checked in this order)
    std::vector<std::string> _index_files_vec{"index.html"};

    // The maximum allowed size (in bytes) of the client request body (default 1MB)
    std::size_t _client_max_body_size{1000000};

    // Enables or disables the directory listing output
    bool _autoindex{false};

    // URI that will be shown for the specified error codes (must be between 300 and 599)
    std::map<int, std::string> _error_pages_map{};

    // ! Should remove because server can listen to multiple host:ports combinations
    std::string _host;

    // ! Should remove because server can listen to multiple host:ports combinations
    int _port;

    // Sets configuration depending on a request URI
    // TODO std::map<std::string, LocationConfig> _locations_map;

    // TODO: CGI handler, maps extensions (e.g., `.py` or `.php`) to their interpreters
    // (`/usr/bin/python3`) std::map<std::string, std::string> _cgi_handler;

private: // Data members for parser only
    // Represents whether a value has already been seen in the config file
    bool _seen_root{false};
    bool _seen_client_max_body_size{false};
    bool _seen_autoindex{false};

    // Represents whether the current value was inherited or specified in this context
    bool _using_parent_index_files_vec{true};
    bool _using_parent_error_pages_map{true};

    // `LocationConfig`s in string form only for use in parser
    std::vector<std::string> _locationConfigsStr{};

private: // Member functions for parser only
    // Main parser
    void parseServerConfig(std::string server_block_str);
    // Helper used by parser
    void setConfigurationValue(std::string directive);

    // Setters don't need to be public
    void setListen(std::string directive);
    void setServerName(std::string directive);
    void setRoot(std::string directive);
    void setClientMaxBodySize(std::string directive);
    void setAutoIndex(std::string directive);
    void setErrorPage(std::string directive);
    void setIndex(std::string directive);
};
