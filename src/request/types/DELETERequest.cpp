#include "DELETERequest.hpp"
#include "Server.hpp"

DELETERequest::DELETERequest(HTTPRequestData data, const LocationConfig* location_config) :
    HTTPRequest(data, location_config)
{}

// ? not sure if we need to handle DELETE in CGI
void DELETERequest::generateResponse(Server* server, int clientFd)
{
    _server = server;
    _clientFd = clientFd;
    _clientData = &(_server->getClientDataMap()[_clientFd]);

    // Check if we are generating brand new response or continuing previous
    if (_responseState != NOT_STARTED)
        return continuePrevious();

    _responseState = IN_PROGRESS;

    // Check method allowed
    if (!_effective_config->getLimitExcept().empty() &&
        _effective_config->getLimitExcept().find("delete") == _effective_config->getLimitExcept().end())
    {
        return errorResponse(405);
    }

    // Redirection handling
    if (_effective_config->getReturn().first != -1)
        return handleRedirection(_effective_config->getReturn());

    // Resolve target resource path
    std::filesystem::path targetPath{_effective_config->getRoot()};
    targetPath /= getURInoLeadingSlash();

    // Prevent escaping root
    std::filesystem::path safePath;
    if (!normalizeAndValidateUnderRoot(targetPath, safePath))
        return errorResponse(403);

    if (!std::filesystem::exists(safePath))
        return errorResponse(404);

    if (std::filesystem::is_directory(safePath))
        return errorResponse(403);

    // Remove the file
    std::error_code ec;
    bool removed = std::filesystem::remove(safePath, ec);
    if (ec)
        return errorResponse(500);

    if (!removed)
        return errorResponse(404);

    ResponseWriter response(200, {{"Content-Type", "text/plain"}}, std::string("Deleted \"") + safePath.filename().string() + "\"\n");
    _fullResponse = response.write();
    _responseState = READY;
}

void DELETERequest::continuePrevious()
{
    std::size_t num_ready{0};
    for (auto& [fileFd, fileData] : _clientData->openFiles)
    {
        if (fileData.finished)
        {
            if (fileData.fileType == OpenFile::READ)
            {
                ++num_ready;
                if (_responseWithoutBody)
                {
                    // ? not sure if there is something to do here
                }
            }
            else if (fileData.fileType == OpenFile::WRITE)
            {
                ++num_ready;
                // ? not sure if there is something to do here
            }
        }
    }
    if (num_ready == _clientData->openFiles.size())
        _responseState = READY;
}
