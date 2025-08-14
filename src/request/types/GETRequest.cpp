#include "GETRequest.hpp"

GETRequest::GETRequest(HTTPRequestData data, const LocationConfig *location_config)
    : HTTPRequest(data, location_config)
{
}

void GETRequest::generateResponse(Server* server, int clientFd)
{
    _server = server;
    _clientFd = clientFd;
    _clientData = &(_server->getClientDataMap()[_clientFd]);

    // Check if we are generating brand new response or continuing previous
    if (_responseState != NOT_STARTED)
        return continuePrevious();

    _responseState = IN_PROGRESS;

    // Check if GET requests for this URI are allowed
    if (!_effective_config->getLimitExcept().empty() &&
        _effective_config->getLimitExcept().find("get") == _effective_config->getLimitExcept().end())
    {
        // Method not allowed
        return errorResponse(405);
    }

    // Check if URI is marked for redirection
    if (_effective_config->getReturn().first != -1)
    {
        return handleRedirection(_effective_config->getReturn());
    }

    std::filesystem::path resourcePath{_effective_config->getRoot()};
    resourcePath /= getURInoLeadingSlash(); // resourcePath = root + requested URI

    // Prevent escaping root
    std::filesystem::path safePath;
    if (!normalizeAndValidateUnderRoot(resourcePath, safePath))
        return errorResponse(403);

    if (std::filesystem::exists(safePath))
    {
        // look for index file. if not found, return either directory listing or error
        if (std::filesystem::is_directory(safePath))
        {
            for (const auto &file : _effective_config->getIndexFilesVec())
            {
                // Join paths: requested URI dir + index file name
                std::filesystem::path curFilePath{safePath / file};
                // Commit to serving this file if it exists, even if permission denied
                if (std::filesystem::exists(curFilePath))
                    return serveFile(curFilePath);
            }
            if (_effective_config->getAutoIndex())
            {
                ResponseWriter response(200, {{"Content-Type", "text/html"}}, getDirectoryListingBody(safePath));
                _fullResponse = response.write();
                _responseState = READY;
                return;
            }
        }
        else
        {
            // requested resource is a file
            return serveFile(safePath);
        }
    }

    // requested resource (file or directory) doesn't exist
    errorResponse(404);
}

void GETRequest::serveFile(const std::filesystem::path &filePath)
{
    for (const auto& [extension, interpreter] : _effective_config->getCGIHandlersMap())
    {
        if (filePath.extension().string() == extension)
            return serveCGI(filePath, interpreter);
    }
    try
    {
        // Will throw if fail to open fd
        openFileSetHeaders(filePath);

        return;

        // Will throw on any read error
        // std::string fileContents{readFileToString(filePath.string())};

        // ResponseWriter response(200, headers, fileContents);
        // return response.write();
    }
    catch (const std::exception &e)
    {
        std::cerr << "File " << _data.uri << " could not be opened: " << e.what() << '\n';
    }
    // Error reading from file
    // RFC says 403 Forbidden can be replaced by 404 Not Found, if desired
    errorResponse(403);
}


void GETRequest::continuePrevious()
{
    std::size_t num_ready{0};
    for (auto& [fileFd, fileData] : _clientData->openFiles)
    {
        if (fileData.finished)
        {
            if (fileData.fileType == OpenFile::READ)
            {
                ++num_ready;
                if (fileData.isCGI && _CgiSubprocess != nullptr)
                {
                    if (_CgiSubprocess->getChildExitStatus() == 0)
                        cgiOutputToResponse(fileData.content);
                    else
                    {
                        _CgiSubprocess->killSubprocess();
                        _responseState = IN_PROGRESS;
                        return errorResponse(500);
                    }
                }
                else if (_responseWithoutBody != nullptr)
                {
                    _responseWithoutBody->setBody(fileData.content);
                    _fullResponse = _responseWithoutBody->write();
                }
            }
            else if (fileData.fileType == OpenFile::WRITE)
            {
                ++num_ready;
                // ? nothing to do?
            }
        }
    }
    if (num_ready == _clientData->openFiles.size())
        _responseState = READY;
}
