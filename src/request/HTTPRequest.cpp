#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest(HTTPRequestData data, const LocationConfig *location_config)
    : _data(std::move(data))
    , _effective_config(location_config)
{
}

bool HTTPRequest::isCloseConnection() const
{
    return _data.headers.find("Connection") != _data.headers.end() && _data.headers.at("Connection") == "close";
}

std::string HTTPRequest::getURInoLeadingSlash() const
{
    std::string result{_data.uri};

    if (result.length() && result[0] == '/')
        result.erase(result.begin());

    return result;
}

std::string HTTPRequest::getErrorResponseBody(int errorCode) const
{
    // The default minimal response body
    std::string result{getMinimalErrorDefaultBody(errorCode)};

    try
    {
        // Will throw if no error page is not defined
        std::string error_file{_effective_config->getErrorPagesMap().at(404)};

        // Will throw if file could not be opened
        result = readFileToString(error_file);
    }
    catch (const std::out_of_range &)
    {
        std::cerr << "Custom error page for " << errorCode << " not defined. Returning default error response body." << '\n';
    }
    catch (const std::exception &)
    {
        std::cerr << "Custom error page file for " << errorCode << " could not be opened. Returning default error response body." << '\n';
    }

    return result;
}

std::string HTTPRequest::getMinimalErrorDefaultBody(int errorCode) const
{
    switch (errorCode)
    {
    case 400:
        return "<html><head><title>500 Internal Server Error</title></head>"
               "<body><h1>500 Internal Server Error</h1>"
               "<p>The server encountered an internal error and was unable to complete your request.</p></body></html>";
    case 403:
        return "<html><head><title>403 Forbidden</title></head>"
               "<body><h1>403 Forbidden</h1><p>Access to resource is forbidden.</p></body></html>";
    case 404:
        return "<html><head><title>404 Not Found</title></head>"
               "<body><h1>404 Not Found</h1><p>The requested URL was not found on this server.</p></body></html>";
    case 405:
        return "<html><head><title>405 Method Not Allowed</title></head>"
               "<body><h1>405 Method Not Allowed</h1><p>Request method not supported by the requested "
               "resource.</p></body></html>";
    case 413:
        return "<html><head><title>413 Payload Too Large</title></head>"
               "<body><h1>413 Payload Too Large</h1><p>The request entity is too large.</p></body></html>";
    case 415:
        return "<html><head><title>415 Unsupported Media Type</title></head>"
               "<body><h1>415 Unsupported Media Type</h1><p>The server does not support the requested media "
               "type.</p></body></html>";
    case 501:
        return "<html><head><title>501 Not Implemented</title></head>"
               "<body><h1>501 Not Implemented</h1><p>The server does not support the facility "
               "required.</p></body></html>";
    case 500:
        return "<html><head><title>500 Internal Server Error</title></head>"
               "<body><h1>500 Internal Server Error</h1>"
               "<p>The server encountered an internal error and was unable to complete your request.</p></body></html>";
    default: // Fallback to 500
        return "<html><head><title>500 Internal Server Error</title></head>"
               "<body><h1>500 Internal Server Error</h1>"
               "<p>The server encountered an internal error and was unable to complete your request.</p></body></html>";
    }
}

// Helper function for getDirectoryListingBody
std::vector<HTTPRequest::FileInfo> listDirectory(const std::filesystem::path &dir_path)
{
    std::vector<HTTPRequest::FileInfo> files;

    try
    {
        // Check if directory exists
        if (!std::filesystem::exists(dir_path) || !std::filesystem::is_directory(dir_path))
        {
            std::cerr << "Directory does not exist or is not a directory: " << dir_path << std::endl;
            return files;
        }

        // Iterate through directory entries
        for (const auto &entry : std::filesystem::directory_iterator(dir_path))
        {
            HTTPRequest::FileInfo info;
            info.name = entry.path().filename().string();
            info.is_directory = entry.is_directory();

            // Get file size (0 for directories)
            if (info.is_directory)
            {
                info.size = 0;
            }
            else
            {
                try
                {
                    info.size = entry.file_size();
                }
                catch (const std::filesystem::filesystem_error &)
                {
                    info.size = 0; // If can't get size
                }
            }

            // Get last modification time
            try
            {
                auto ftime = entry.last_write_time();
                // Convert to system_clock time_point
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
                info.modified_time = std::chrono::system_clock::to_time_t(sctp);
            }
            catch (const std::filesystem::filesystem_error &)
            {
                info.modified_time = 0; // If can't get time
            }

            files.push_back(info);
        }

        // Sort files: directories first, then by name
        std::sort(files.begin(), files.end(),
                  [](const HTTPRequest::FileInfo &a, const HTTPRequest::FileInfo &b)
                  {
                      if (a.is_directory != b.is_directory)
                      {
                          return a.is_directory; // directories first
                      }
                      return a.name < b.name; // then alphabetically
                  });
    }
    catch (const std::exception &e)
    {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }

    return files;
}

std::string HTTPRequest::getDirectoryListingBody(const std::filesystem::path &dirPath) const
{
    auto filesVec = listDirectory(dirPath);

    if (filesVec.empty())
        return getErrorResponseBody(404); // Should never happen but just to be safe

    std::string html = "<!DOCTYPE html>\n<html>\n<head>\n";
    html += "<title>Index of " + _data.uri + "</title>\n";
    html += "<style>\n";
    html += "body { font-family: Arial, sans-serif; margin: 20px; }\n";
    html += "table { border-collapse: collapse; width: 100%; }\n";
    html += "th, td { text-align: left; padding: 8px; border-bottom: 1px solid #ddd; }\n";
    html += "th { background-color: #f2f2f2; }\n";
    html += "a { text-decoration: none; color: #0066cc; }\n";
    html += "a:hover { text-decoration: underline; }\n";
    html += ".dir { font-weight: bold; }\n";
    html += "</style>\n</head>\n<body>\n";

    html += "<h1>Index of " + _data.uri + "</h1>\n";
    html += "<table>\n<tr><th>Name</th><th>Size</th><th>Last Modified</th></tr>\n";

    // Add parent directory link if not root
    if (_data.uri != "/")
    {
        html += "<tr><td><a href=\"../\" class=\"dir\">../</a></td><td>-</td><td>-</td></tr>\n";
    }

    for (const auto &file : filesVec)
    {
        html += "<tr><td>";
        auto fileUri{std::filesystem::path(getURInoLeadingSlash()) / file.name};
        if (file.is_directory)
        {
            html += "<a href=\"" + fileUri.string() + "/\" class=\"dir\">" + file.name + "/</a>";
        }
        else
        {
            html += "<a href=\"" + fileUri.string() + "\">" + file.name + "</a>";
        }
        html += "</td><td>";

        if (file.is_directory)
        {
            html += "-";
        }
        else
        {
            html += bytesToHumanReadable(file.size);
        }

        html += "</td><td>";
        if (file.modified_time != 0)
        {
            char time_str[100];
            std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&file.modified_time));
            html += time_str;
        }
        else
        {
            html += "Unknown";
        }
        html += "</td></tr>\n";
    }

    html += "</table>\n</body>\n</html>";
    return html;
}

std::string HTTPRequest::handleRedirection(const std::pair<int, std::string> &redirectInfo) const
{
    std::string codeWithReasonPhrase{std::to_string(redirectInfo.first) + " " + reasonPhraseFromStatusCode(redirectInfo.first)};
    std::string responseBody{"<html><head><title>"};
    responseBody += codeWithReasonPhrase;
    responseBody += "</title></head><body><h1>";
    responseBody += codeWithReasonPhrase;
    responseBody += "</h1></body></html>";

    std::unordered_map<std::string, std::string> headers;
    headers["Content-Type"] = "text/html";

    if (!redirectInfo.second.empty())
        headers["Location"] = redirectInfo.second;
    ResponseWriter response(redirectInfo.first, headers, responseBody);

    return response.write();
}

std::string HTTPRequest::getMIMEtype(const std::string &extension) const
{
    if (extension == ".html")
        return "text/html";
    else if (extension == ".htm")
        return "text/html";
    else if (extension == ".css")
        return "text/css";
    else if (extension == ".js")
        return "application/javascript";
    else if (extension == ".json")
        return "application/json";
    else if (extension == ".png")
        return "image/png";
    else if (extension == ".jpg")
        return "image/jpg";
    else if (extension == ".gif")
        return "image/gif";
    else if (extension == ".svg")
        return "image/svg+xml";
    else if (extension == ".pdf")
        return "application/pdf";
    else if (extension == ".txt")
        return "text/plain";
    else if (extension == "")
        return "text/plain"; // for extensionless files
    // ... more mappings can be added
    return "application/octet-stream"; // for unknown extensions
}

std::string HTTPRequest::errorResponse(int errorCode) const
{
    ResponseWriter response(errorCode, {{"Content-Type", "text/html"}}, getErrorResponseBody(errorCode));
    return response.write();
}

std::unordered_map<std::string, std::string> HTTPRequest::createCGIenvironment(const std::filesystem::path &filePathAbs) const
{
    std::unordered_map<std::string, std::string> envMap;

    envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
    envMap["SERVER_PROTOCOL"] = _data.version;
    envMap["REQUEST_METHOD"] = _data.methodStr();
    envMap["REQUEST_URI"] = _data.uri;
    envMap["SCRIPT_FILENAME"] = filePathAbs.string();

    std::string virtualScriptPath{_data.uri};
    std::string queryStr{""};
    std::size_t queryBegin{virtualScriptPath.find_last_of('?')}; // technically this is 1 before query begin
    if (queryBegin != std::string::npos)
    {
        if (queryBegin + 1 < virtualScriptPath.length())
            queryStr = virtualScriptPath.substr(queryBegin + 1); // get part after '?'
        virtualScriptPath.erase(queryBegin);                     // erase from '?' to end of string
    }
    envMap["SCRIPT_NAME"] = virtualScriptPath;
    envMap["QUERY_STRING"] = queryStr;

    if (_data.headers.find("content-type") != _data.headers.end())
        envMap["CONTENT_TYPE"] = _data.headers.at("content-type");
    else
        envMap["CONTENT_TYPE"] = "";

    envMap["CONTENT_LENGTH"] = _data.body.length();

    if (_data.headers.find("host") != _data.headers.end())
        envMap["SERVER_NAME"] = _data.headers.at("host");
    else
        envMap["SERVER_NAME"] = "";

    // ! SERVER_PORT (port the server received the request on) and REMOTE_ADDR (IP address of client) are
    // ! not added to the environment since they are not available in HTTPRequestData _data

    return envMap;
}

std::string HTTPRequest::serveCGI(const std::filesystem::path &filePath, const std::string &interpreter) const
{
    if (!std::filesystem::exists(filePath))
        return errorResponse(404);

    auto filePathAbs{std::filesystem::absolute(filePath)};
    try
    {
        CGISubprocess subprocess;
        subprocess.setEnvironment(createCGIenvironment(filePathAbs));
        subprocess.createSubprocess(filePathAbs, interpreter);
        subprocess.writeToChild(_data.body);                // ! blocking
        std::string cgi_output{subprocess.readFromChild()}; // ! blocking
        subprocess.waitChild();                             // uses WNOHANG

        // TODO: before returning, parse CGI-returned response and add any missing headers and status OK, etc.
        return cgi_output;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return errorResponse(500);
    }
}

bool HTTPRequest::normalizeAndValidateUnderRoot(const std::filesystem::path &candidate, std::filesystem::path &outNormalized) const
{
    std::filesystem::path rootPath = std::filesystem::path(_effective_config->getRoot());
    std::filesystem::path normRoot = rootPath.lexically_normal();
    std::filesystem::path normCand = candidate.lexically_normal();

    auto rootStr = normRoot.string();
    auto candStr = normCand.string();

    // Ensure trailing separator handling: either exact match, or next char is '/'
    bool startsWith = candStr.compare(0, rootStr.size(), rootStr) == 0 &&
                      (candStr.size() == rootStr.size() || candStr[rootStr.size()] == '/');
    if (!startsWith)
        return false;

    outNormalized = normCand;
    return true;
}
