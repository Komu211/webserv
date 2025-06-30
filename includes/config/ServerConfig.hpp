#pragma once

#include <map>
#include <string>
#include <vector>

class GlobalConfig;
class LocationConfig;

class ServerConfig
{
private:
    // ! Should remove because server can listen to multiple host:ports combinations
    std::string _host;

    // ! Should remove because server can listen to multiple host:ports combinations
    int _port;

    // All `host:port` combinations this server listens to (in their colon-separated string form)
    std::vector<std::string> _listen_host_port{};

    // All `host:port` combinations this server listens to (in their addrinfo form)
    std::vector<struct addrinfo *> _addrinfo_lists_vec{};

    // ! Should remove because there can be multiple index files
    std::string _index;

    // Root directory for requests to this server
    std::string _root{"./html"};

    // All `server_name`s (Host header) this server responds to
    std::vector<std::string> _serverNames;

    // Files that will be used as an index (checked in this order)
    std::vector<std::string> _index_files_vec{};

    // Sets configuration depending on a request URI
    // * std::map<std::string, LocationConfig> _locations_map;

    // URI that will be shown for the specified error codes (must be between 300 and 599)
    std::map<int, std::string> _error_pages_map{};

    // The maximum allowed size (in bytes) of the client request body (default 1MB)
    std::size_t _client_max_body_size{1000000};

    // Enables or disables the directory listing output
    bool _autoindex{false};

    // TODO: CGI handler, maps extensions (e.g., `.py` or `.php`) to their interpreters
    // (`/usr/bin/python3`) std::map<std::string, std::string> _cgi_handler;

public:
    ServerConfig(); // * Just for testing; should mark as `=delete` later
    [[nodiscard]] std::string              getHost() const;  // !
    [[nodiscard]] int                      getPort() const;  // !
    [[nodiscard]] std::string              getIndex() const; // !
    [[nodiscard]] std::string              getRoot() const;
    [[nodiscard]] std::vector<std::string> getServerNames() const;

    void setHost(const std::string &host);   // !
    void setIndex(const std::string &index); // !
    void setRoot(const std::string &root);
    void setPort(int newPort); // !
    void setServerNames(const std::vector<std::string> &serverNames);
    void addServerName(const std::string &serverName);

public:
    // OCF
    ServerConfig(const ServerConfig &other) = default;
    ServerConfig &operator=(const ServerConfig &other) = default;
    ServerConfig &operator=(ServerConfig &&other) = default;
    ~ServerConfig() = default;

    // Main parameterized ctor (parses server_block_str, inherits the rest from global_config)
    ServerConfig(const std::string &server_block_str, const GlobalConfig &global_config);
};
