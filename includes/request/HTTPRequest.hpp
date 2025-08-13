#pragma once

#include "HTTPRequestData.hpp"
#include "LocationConfig.hpp"
#include "ResponseWriter.hpp"
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

class LocationConfig;
class ResponseWriter;
class HTTPRequest
{
protected:
    HTTPRequestData       _data;
    const LocationConfig *_effective_config;

protected: // helper functions to use within public member functions of inherited classes
    // Remove leading slash from URI so std::filesystem doesn't think it refers to root directory
    std::string getURInoLeadingSlash() const;
    // Get 404 response either from a custom 404 file or a minimal default
    std::string getErrorResponseBody(int errorCode) const;
    // If custom files for error codes are not defined, these default bodies are used
    std::string getMinimalErrorDefaultBody(int errorCode) const;
    // Create HTML directory listing of a given URI (it should be an existing directory)
    std::string getDirectoryListingBody(const std::filesystem::path &dirPath) const;
    // Determine "Content-Type" header based on a given file extension
    std::string getMIMEtype(const std::string &extension) const;
    // Redirection
    std::string handleRedirection(const std::pair<int, std::string> &redirectInfo) const;
    // Returning error response based on the provided code
    std::string errorResponse(int errorCode) const;

    // Normalize path and validate it is under root
    bool normalizeAndValidateUnderRoot(const std::filesystem::path &candidate, std::filesystem::path &outNormalized) const;

public:
    // Struct for use in directory listing
    struct FileInfo
    {
        std::string name;
        bool        is_directory;
        std::size_t size;
        std::time_t modified_time;
    };

public:
    HTTPRequest() = delete;
    explicit HTTPRequest(HTTPRequestData data, const LocationConfig *location_config);
    virtual ~HTTPRequest() = default;

    // Where the magic happens
    virtual std::string handle() = 0;

    [[nodiscard]] bool isCloseConnection() const;
};
