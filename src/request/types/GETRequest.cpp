#include "GETRequest.hpp"

GETRequest::GETRequest(HTTPRequestData data, const LocationConfig *location_config)
    : HTTPRequest(data, location_config)
{
}

std::string GETRequest::handle()
{
    // Check if GET requests for this URI are allowed
    if (!_effective_config->getLimitExcept().empty() &&
        _effective_config->getLimitExcept().find("get") == _effective_config->getLimitExcept().end())
    {
        // Method not allowed
        ResponseWriter response(405, {{"Content-Type", "text/html"}}, getErrorResponseBody(405));
        return response.write();
    }

    // Check if URI is marked for redirection
    if (_effective_config->getReturn().first != -1)
    {
        return handleRedirection(_effective_config->getReturn());
    }

    std::filesystem::path resourcePath{_effective_config->getRoot()};
    resourcePath /= getURInoLeadingSlash(); // resourcePath = root + requested URI

    if (std::filesystem::exists(resourcePath))
    {
        // look for index file. if not found, return either directory listing or error
        if (std::filesystem::is_directory(resourcePath))
        {
            for (const auto &file : _effective_config->getIndexFilesVec())
            {
                std::filesystem::path curFilePath{resourcePath / file};
                try
                {
                    // Will throw for non-existent file or other reading error
                    // Can also check permissions to return 403 Forbidden, but not necessary
                    std::string fileContents{readFileToString(curFilePath.string())};

                    std::unordered_map<std::string, std::string> headers;
                    headers["Content-Type"] = getMIMEtype(curFilePath.extension().string());
                    headers["Last-Modified"] = getLastModTimeHTTP(curFilePath);

                    ResponseWriter response(200, headers, fileContents);
                    return response.write();
                }
                catch (const std::exception &)
                {
                    std::cerr << "Index file " << file << " either does not exist or could not be opened." << '\n';
                }
            }
            if (_effective_config->getAutoIndex())
            {
                ResponseWriter response(200, {{"Content-Type", "text/html"}}, getDirectoryListingBody(resourcePath));
                return response.write();
            }
        }
        else
        {
            // requested resource is a file
            try
            {
                // Will throw for non-existent file or other reading error
                std::string fileContents{readFileToString(resourcePath.string())};

                std::unordered_map<std::string, std::string> headers;
                headers["Content-Type"] = getMIMEtype(resourcePath.extension().string());
                headers["Last-Modified"] = getLastModTimeHTTP(resourcePath);

                ResponseWriter response(200, headers, fileContents);
                return response.write();
            }
            catch (const std::exception &)
            {
                std::cerr << "File " << _data.uri << " either does not exist or could not be opened." << '\n';
            }
        }
    }

    // requested resource (file or directory) doesn't exist
    ResponseWriter response(404, {{"Content-Type", "text/html"}}, getErrorResponseBody(404));
    return response.write();
}