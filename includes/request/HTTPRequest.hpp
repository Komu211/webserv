#pragma once

#include "CGISubprocess.hpp"
#include "HTTPRequestData.hpp"
#include "LocationConfig.hpp"
#include "ResponseWriter.hpp"
#include "PollManager.hpp"
#include "Server.hpp"
#include "HTTPRequestParser.hpp"
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <memory>
#include <unordered_set>
#include <sstream>
#include "utils.hpp"

// Forward declarations
class Server;
class LocationConfig;
class ResponseWriter;
class PollManager;
class HTTPRequestParser;
struct OpenFile;
struct ClientData;

class HTTPRequest
{
protected:
    enum ResponseState
    {
        NOT_STARTED,
        IN_PROGRESS,
        READY
    };

    HTTPRequestData       _data;
    const LocationConfig *_effective_config;
    ResponseState _responseState{NOT_STARTED};
    // ResponseWriter* responseWithoutBody{nullptr};
    std::unique_ptr<ResponseWriter> _responseWithoutBody{nullptr};
    std::unique_ptr<CGISubprocess> _CgiSubprocess{nullptr};
    std::string _fullResponse;
    Server* _server;
    int _clientFd;
    ClientData* _clientData;

protected: // helper functions to use within public member functions of inherited classes
    // Remove leading slash from URI so std::filesystem doesn't think it refers to root directory
    std::string               getURInoLeadingSlash() const;
    // Get 404 response either from a custom 404 file or a minimal default
    // std::string               getErrorResponseBody(int errorCode) const; // ! delete
    // If custom files for error codes are not defined, these default bodies are used
    std::string               getMinimalErrorDefaultBody(int errorCode) const;
    // Create HTML directory listing of a given URI (it should be an existing directory)
    std::string               getDirectoryListingBody(const std::filesystem::path &dirPath) const;
    // Determine "Content-Type" header based on a given file extension
    std::string               getMIMEtype(const std::string &extension) const;
    // Redirection
    void handleRedirection(const std::pair<int, std::string> &redirectInfo);
    // Returning error response based on the provided code
    // [[nodiscard]] std::string errorResponse(int errorCode) const; // ! delete
    // Handle CGI and return the full response to be sent to client
    void serveCGI(const std::filesystem::path &filePath, const std::string &interpreter);
    // Create environment variables for CGI subprocess // ! skips SERVER_PORT and REMOTE_ADDR since they are not in HTTPRequestData
    [[nodiscard]] std::unordered_map<std::string, std::string> createCGIenvironment(const std::filesystem::path &filePath) const;

    void errorResponse(int errorCode);
    // bool errorResponseRequiresReadingFile(int errorCode);
    // Opens an fd for file, adds it to poll manager, initalizes _responseWithoutBody, throws on open error
    void openFileSetHeaders(const std::filesystem::path &filePath);
    // Converts the CGI output to a final response ready to be sent to client
    void cgiOutputToResponse(const std::string& cgi_output);

    // Normalize path and validate it is under root
    bool normalizeAndValidateUnderRoot(const std::filesystem::path &candidate, std::filesystem::path &outNormalized) const;

    virtual void continuePrevious() = 0;

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
    HTTPRequest(const HTTPRequest &) = delete;
    HTTPRequest(HTTPRequest &&) = delete;
    virtual ~HTTPRequest() = default;

    // Where the magic happens
    virtual std::string getFullResponse();
    bool fullResponseIsReady();
    virtual void generateResponse(Server* server, int clientFd) = 0;

    [[nodiscard]] bool isCloseConnection() const;
};
