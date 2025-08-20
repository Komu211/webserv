#pragma once

#include "GlobalConfig.hpp"
#include "LocationConfig.hpp"
#include "utils.hpp"
#include <cstring> /* std::memset() */
#include <map>
#include <memory>  /* std::unique_ptr */
#include <netdb.h> /* getaddrinfo(), freeaddrinfo() */
#include <stdexcept>
#include <string>
#include <unistd.h> /* access() */
#include <utility>  /* std::pair */
#include <vector>
// #include <unordered_set> /* Better use unordered_set for _listen_host_port and _serverNames */

// A pair of strings (used for host:port combinations)
using StringPair = std::pair<std::string, std::string>;
// Pair of `struct addrinfo` and human-readable host:port strings
using AddrInfoPair = std::pair<struct addrinfo, StringPair>;

class GlobalConfig;
class LocationConfig;

class ServerConfig
{
public:
    // Main parameterized ctor (parses server_block_str, inherits the rest from global_config)
    ServerConfig(const std::string &server_block_str, const GlobalConfig &global_config);

    // OCF
    ServerConfig() = delete;
    ServerConfig(const ServerConfig &other) = delete;
    ServerConfig(ServerConfig &&other) = default;
    ServerConfig &operator=(const ServerConfig &other) = delete;
    ServerConfig &operator=(ServerConfig &&other) = default;
    ~ServerConfig();

public:
    [[nodiscard]] const std::vector<StringPair>                                &getHostPortPairs() const;
    [[nodiscard]] const std::vector<AddrInfoPair>                              &getAddrInfoVec() const;
    [[nodiscard]] const std::string                                            &getRoot() const;
    [[nodiscard]] const std::vector<std::string>                               &getServerNames() const;
    [[nodiscard]] const std::vector<std::string>                               &getIndexFilesVec() const;
    [[nodiscard]] std::size_t                                                   getClientMaxBodySize() const;
    [[nodiscard]] bool                                                          getAutoIndex() const;
    [[nodiscard]] const std::map<int, std::string>                             &getErrorPagesMap() const;
    [[nodiscard]] const std::map<std::string, std::unique_ptr<LocationConfig>> &getLocationsMap() const;
    [[nodiscard]] const std::map<std::string, std::string>                     &getCGIHandlersMap() const;

private:
    // All `host:port` combinations this server listens to // * Better convert to unordered_set or unordered_map
    std::vector<StringPair> _listen_host_port{{"0.0.0.0", "80"}};

    // All `host:port` combinations this server listens to (in their addrinfo form)
    std::vector<AddrInfoPair> _addrinfo_vec{};

    // All `server_name`s (Host header) this server responds to // * Better convert to unordered_set
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

    // Sets configuration depending on a request URI
    std::map<std::string, std::unique_ptr<LocationConfig>> _locations_map;

    // CGI handler, maps extensions (e.g., `.py` or `.php`) to their interpreters (e.g., `/usr/bin/python3`)
    std::map<std::string, std::string> _cgi_handlers_map{};

private: // Data members for parser only
    // Represents whether a value has already been seen in the config file
    bool _seen_listen{false};
    bool _seen_root{false};
    bool _seen_client_max_body_size{false};
    bool _seen_autoindex{false};
    bool _seen_index{false};
    bool _seen_cgi_handler{false};
    // bool _seen_error_page{false}; unused for now

    // `LocationConfig`s in string form only for use in parser
    std::vector<std::string> _locationConfigsStr{};

    // To be freed at destruction
    std::vector<struct addrinfo *> _addrinfo_lists_vec{};

private: // Member functions for parser only
    // Main parser
    void parseServerConfig(std::string server_block_str);
    // Helpers used by parser
    void setConfigurationValue(std::string directive);
    void setAddrInfo();
    void initLocationConfig();

    // Setters don't need to be public
    void setListen(std::string directive);
    void setServerName(std::string directive);
    void setRoot(std::string directive);
    void setClientMaxBodySize(std::string directive);
    void setAutoIndex(std::string directive);
    void setErrorPage(std::string directive);
    void setIndex(std::string directive);
    void setCGIHandler(std::string directive);
};
