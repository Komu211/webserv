#pragma once

#include <map>
#include <string>
#include <vector>

class LocationConfig
{
private:
    // Root directory for requests to this server
    std::string _root{"./html"};

    // Files that will be used as an index (checked in this order)
    std::vector<std::string> _index_files_vec{};

    // URI that will be shown for the specified error codes (must be between 300 and 599)
    std::map<int, std::string> _error_pages_map{};

    // The maximum allowed size (in bytes) of the client request body (default 1MB)
    std::size_t _client_max_body_size{1000000};

    // Enables or disables the directory listing output
    bool _autoindex{false};

    // TODO: CGI handler, maps extensions (e.g., `.py` or `.php`) to their interpreters
    // (`/usr/bin/python3`) std::map<std::string, std::string> _cgi_handler;

    // ! Add limit_except, upload_store, and return

public:
    // OCF
    LocationConfig() = delete;
    LocationConfig(const LocationConfig &other) = default;
    LocationConfig &operator=(const LocationConfig &other) = default;
    LocationConfig &operator=(LocationConfig &&other) = default;
    ~LocationConfig() = default;

    // Main parameterized constructor
    LocationConfig(const std::string &confStr);

    // ! Getters

    // No setters needed
};