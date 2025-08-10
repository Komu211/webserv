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

std::string HTTPRequest::getDirectoryListingBody(const std::filesystem::path& dirPath) const
{
    if (!std::filesystem::exists(dirPath))
        return getErrorResponseBody(404); // Should never happen but just to be safe

    std::string html_body {"<!DOCTYPE html>\n<html>\n<head>\n"};
    html_body += "<title>Index of " + dirPath.string() + "</title>\n";

    // TODO: Incomplete function

    return html_body;
}

std::string HTTPRequest::handleRedirection(const std::pair<int, std::string> &redirectInfo) const
{
    std::string codeWithReasonPhrase {std::to_string(redirectInfo.first) + " " + reasonPhraseFromStatusCode(redirectInfo.first)};
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

std::string HTTPRequest::getMIMEtype(const std::string& extension) const
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
