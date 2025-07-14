#include "HTTPRequestParser.hpp"

bool HTTPRequestParser::isValidRequest(const std::string &buffer)
{
    auto bodyStart = buffer.find("\r\n\r\n");
    if (bodyStart == std::string::npos)
        return false;
    std::istringstream headerStream(buffer.substr(0, bodyStart));
    bodyStart += 4; // Skip CRLF CRLF
    auto headers = parseHeaders(std::move(headerStream));
    if (headers.find("Content-Length") != headers.end())
    {
        auto contentLength = std::stoul(headers["Content-Length"]);
        if (bodyStart + contentLength > buffer.size())
            return false;
    }
    if (headers.find("Transfer-encoding") != headers.end()
        && headers["Transfer-encoding"] == "chunked")
        return buffer.find("0\r\n\r\n", bodyStart) != std::string::npos;
    return true;
}

HTTPRequestData HTTPRequestParser::parse(const std::string &request_str)
{
    (void)request_str; // Placeholder for actual parsing logic
    // TODO: Implement parsing logic
    HTTPRequestData data = {
        GET,
        "/",
        "1.1",
        {},
        ""
    };
    return data;
}

std::unordered_map<std::string, std::string> HTTPRequestParser::parseHeaders(
    std::istringstream headerStream)
{
    std::unordered_map<std::string, std::string> headers;
    std::string line;

    while (std::getline(headerStream, line)) {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            trim(key);
            trim(value);
            headers[key] = value;
        }
    }
    return headers;
}