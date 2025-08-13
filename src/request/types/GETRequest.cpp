#include "GETRequest.hpp"

GETRequest::GETRequest(HTTPRequestData data, const LocationConfig *location_config)
    : HTTPRequest(data, location_config)
{
}

std::string GETRequest::getFullResponse()
{
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
                return response.write();
            }
        }
        else
        {
            // requested resource is a file
            return serveFile(safePath);
        }
    }

    // requested resource (file or directory) doesn't exist
    return errorResponse(404);
}

std::string GETRequest::serveFile(const std::filesystem::path &filePath) const
{
    for (const auto& [extension, interpreter] : _effective_config->getCGIHandlersMap())
    {
        if (filePath.extension().string() == extension)
            return serveCGI(filePath, interpreter);
    }
    try
    {
        // Will throw on any read error
        std::string fileContents{readFileToString(filePath.string())};

        std::unordered_map<std::string, std::string> headers;
        headers["Content-Type"] = getMIMEtype(filePath.extension().string());
        headers["Last-Modified"] = getLastModTimeHTTP(filePath);

        ResponseWriter response(200, headers, fileContents);
        return response.write();
    }
    catch (const std::exception &)
    {
        std::cerr << "File " << _data.uri << " could not be opened." << '\n';
    }
    // Error reading from file
    // RFC says 403 Forbidden can be replaced by 404 Not Found, if desired
    return errorResponse(403);
}
