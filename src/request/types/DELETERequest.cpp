#include "DELETERequest.hpp"
#include "Server.hpp"

DELETERequest::DELETERequest(HTTPRequestData data, const LocationConfig *location_config)
    : HTTPRequest(data, location_config)
{
}

// TODO: we need to handle DELETE in CGI
void DELETERequest::generateResponse(Server *server, int clientFd)
{
    _server = server;
    _clientFd = clientFd;
    _clientData = &(_server->getClientDataMap()[_clientFd]);

    // Check if we are generating brand new response or continuing previous
    if (_responseState != NOT_STARTED)
        return continuePrevious();

    _responseState = IN_PROGRESS;

    // Check method allowed
    // By default, DELETE is not allowed in any location. It must be be specified in config file
    if (_effective_config->getLimitExcept().find("delete") == _effective_config->getLimitExcept().end())
    {
        return errorResponse(405);
    }

    // Redirection handling
    if (_effective_config->getReturn().first != -1)
        return handleRedirection(_effective_config->getReturn());

    // Resolve target resource path
    std::filesystem::path targetPath{_effective_config->getRoot()};
    targetPath /= splitUriIntoPathAndQuery(getURInoLeadingSlash()).first; // get rid of query string

    // Prevent escaping root
    std::filesystem::path safePath;
    if (!normalizeAndValidateUnderRoot(targetPath, safePath))
        return errorResponse(403);

    if (!std::filesystem::exists(safePath))
        return errorResponse(404);

    // If path exists and is a directory, check for index files
    std::filesystem::path cgiPath = safePath;
    if (std::filesystem::is_directory(safePath))
    {
        for (const auto &file : _effective_config->getIndexFilesVec())
        {
            std::filesystem::path indexPath = safePath / file;
            if (std::filesystem::exists(indexPath))
            {
                cgiPath = indexPath;
                break;
            }
        }
    }

    // Check if path matches any CGI handler
    for (const auto &[extension, interpreter] : _effective_config->getCGIHandlersMap())
    {
        if (cgiPath.extension().string() == extension)
        {
            std::cout << "CGI detected for: " << cgiPath << std::endl;
            return serveCGI(cgiPath, interpreter);
        }
    }

    // Don't allow deletion of directories
    if (std::filesystem::is_directory(safePath))
        return errorResponse(403);

    // Remove the file
    std::error_code ec;
    bool            removed = std::filesystem::remove(safePath, ec);
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
    for (auto &[fileFd, fileData] : _clientData->openFiles)
    {
        if (fileData.finished)
        {
            if (fileData.fileType == OpenFile::READ)
            {
                ++num_ready;
                if (fileData.isCGI && _CgiSubprocess != nullptr)
                    cgiOutputToResponse(fileData.content);
                else if (_responseWithoutBody != nullptr)
                {
                    // non-CGI read
                    _responseWithoutBody->setBody(fileData.content);
                    _fullResponse = _responseWithoutBody->write();
                }
            }
            else if (fileData.fileType == OpenFile::WRITE)
            {
                ++num_ready;
                // nothing to do
            }
        }
    }
    if (num_ready == _clientData->openFiles.size())
    {
        if (_CgiStartTime.has_value()) // A CGI process exists
            return checkCGIstatus();
        else // No CGI process exists
            _responseState = READY;
    }
}
