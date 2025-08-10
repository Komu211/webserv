#include "GETRequest.hpp"

GETRequest::GETRequest(HTTPRequestData data, const LocationConfig* location_config) :
    HTTPRequest(data, location_config)
{}

std::string GETRequest::handle()
{
    std::filesystem::path resourcePath {_effective_config->getRoot()};
    resourcePath /= getURInoLeadingSlash(); // resourcePath = root + requested URI

    if (std::filesystem::exists(resourcePath))
    {
        // look for index file. if not found, return either directory listing or error
        if (std::filesystem::is_directory(resourcePath))
        {
            for (const auto& file : _effective_config->getIndexFilesVec())
            {
                std::filesystem::path curFilePath{resourcePath / file};
                try
                {
                    // Will throw for non-existent file or other reading error
                    std::string fileContents {readFileToString(curFilePath.string())};

                    std::unordered_map<std::string, std::string> headers;
                    headers["Content-Type"] = getMIMEtype(curFilePath.extension().string());
                    headers["Last-Modified"] = getLastModTimeHTTP(curFilePath);

                    ResponseWriter response(200, headers, fileContents);
                    return response.write();
                }
                catch(const std::exception&)
                {
                    std::cerr << "Index file " << file << " either does not exist or could not be opened." << '\n';
                }
            }
            if (_effective_config->getAutoIndex())
            {
                ResponseWriter response(200, {{"Content-Type", "text/html"}}, getDirectoryListingBody(resourcePath));
            }
        }
    }
    // requested resource (file or directory) doesn't exist
    ResponseWriter response(404, {{"Content-Type", "text/html"}}, getErrorResponseBody(400));
    return response.write();


    // Create response body string (either file contents, directory listing, or error)
    // Create std::unordered_map<std::string, std::string> with headers such as "Content-Type" and "Last-Modified"
    // Determine response code
    // Pass the above three to create ResponseWriter object
    // Return responseWriter.write()

    // Replace this with actual logic for handling GET requests
    // return "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\ncontent-length: 22\r\n\r\nHello, World from GET!";
}