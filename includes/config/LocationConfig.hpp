#pragma once

#include "ServerConfig.hpp"
#include <map>
#include <set>
#include <string>
#include <utility> /* std::pair */
#include <vector>

class ServerConfig;

class LocationConfig
{
public:
    // Main parameterized ctor (parses location_block_str, inherits the rest from server_config)
    LocationConfig(const std::string &location_block_str, const ServerConfig &server_config);

    // OCF
    LocationConfig() = delete;
    LocationConfig(const LocationConfig &other) = delete;
    LocationConfig(LocationConfig &&other) = default;
    LocationConfig &operator=(const LocationConfig &other) = delete;
    LocationConfig &operator=(LocationConfig &&other) = default;
    ~LocationConfig() = default;

public:
    // Getters
    [[nodiscard]] const std::string                 &getRoot() const;
    [[nodiscard]] const std::vector<std::string>    &getIndexFilesVec() const;
    [[nodiscard]] std::size_t                        getClientMaxBodySize() const;
    [[nodiscard]] bool                               getAutoIndex() const;
    [[nodiscard]] const std::map<int, std::string>  &getErrorPagesMap() const;
    // TODO cgi getter
    [[nodiscard]] const std::set<std::string>       &getLimitExcept() const;
    [[nodiscard]] const std::string                 &getUploadStore() const;
    [[nodiscard]] const std::pair<int, std::string> &getReturn() const;

private:
    // Root directory for requests to this location
    std::string _root{"./html"};

    // Files that will be used as an index (checked in this order)
    std::vector<std::string> _index_files_vec{"index.html"};

    // The maximum allowed size (in bytes) of the client request body (default 1MB)
    std::size_t _client_max_body_size{1000000};

    // Enables or disables the directory listing output
    bool _autoindex{false};

    // URI that will be shown for the specified error codes (must be between 300 and 599)
    std::map<int, std::string> _error_pages_map{};

    // TODO: CGI handler, maps extensions (e.g., `.py` or `.php`) to their interpreters
    // (`/usr/bin/python3`) std::map<std::string, std::string> _cgi_handler;

    // Limits allowed HTTP methods inside a location
    std::set<std::string> _limit_except{};

    // Location to save uploads on the server
    std::string _upload_store{"/uploads"};

    // For redirects; Stops processing and returns the specified code to the client
    std::pair<int, std::string> _return{-1, ""};

private: // Data members for parser only
    // Represents whether a value has already been seen in the config file
    bool _seen_root{false};
    bool _seen_client_max_body_size{false};
    bool _seen_autoindex{false};
    bool _seen_index{false};
    bool _seen_error_page{false};
    bool _seen_limit_except{false};
    bool _seen_upload_store{false};
    bool _seen_return{false};

private: // Member functions for parser only
    // Main parser
    void parseLocationConfig(std::string location_block_str);
    // Helpers used by parser
    void setConfigurationValue(std::string directive);

    // Setters don't need to be public
    void setRoot(std::string directive);
    void setClientMaxBodySize(std::string directive);
    void setAutoIndex(std::string directive);
    void setErrorPage(std::string directive);
    void setIndex(std::string directive);
    void setLimitExcept(std::string directive);
    void setUploadStore(std::string directive);
    void setReturn(std::string directive);
    // TODO: CGI handler
};