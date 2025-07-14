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

HTTPRequestData HTTPRequestParser::parse(const std::string &requestStr)
{
    auto bodyStart = requestStr.find("\r\n\r\n");
    std::istringstream headerStream(requestStr.substr(0, bodyStart));
    auto HTTPData = getRequestLine(headerStream);
    HTTPData.headers = parseHeaders(std::move(headerStream));
    bodyStart += 4; // Skip CRLF CRLF
    HTTPData.body = getBody(HTTPData.headers, requestStr.substr(bodyStart));
    return HTTPData;
}

std::string HTTPRequestParser::getBody(const std::unordered_map<std::string, std::string> &headers, const std::string &bodyStr)
{
    auto transferEncodingIt = headers.find("Transfer-Encoding");
    if (transferEncodingIt != headers.end() && transferEncodingIt->second == "chunked")
    {
        return parseChunkedBody(bodyStr);
    }
    auto contentLengthIt = headers.find("Content-Length");
    if (contentLengthIt != headers.end())
    {
        size_t bodyLength = std::stoul(contentLengthIt->second);
        return bodyStr.substr(0, bodyLength);
    }
    return bodyStr;
}

std::string HTTPRequestParser::parseChunkedBody(const std::string &bodyStr)
{
    std::stringstream bodyStream(bodyStr);
    std::string chunkSizeStr;
    std::string chunkData;
    std::string result;

    while (std::getline(bodyStream, chunkSizeStr, '\r')) {
        if (chunkSizeStr.empty())
            break;
        std::getline(bodyStream, chunkData);
        size_t chunkSize = std::stoul(chunkSizeStr, nullptr, 16);
        if (chunkSize == 0)
            break;
        if (chunkData.size() < chunkSize)
            throw std::runtime_error("Invalid chunk size");
        result.append(chunkData.substr(0, chunkSize));
    }
    return result;
}

HTTPRequestData HTTPRequestParser::getRequestLine(std::istringstream &headerStream)
{
    HTTPRequestData HTTPData;
    std::string line;
    std::string method;
    std::getline(headerStream, line);
    if (line.back() == '\r')
        line.pop_back();
    std::istringstream lineStream(line);
    lineStream >> method >> HTTPData.uri >> HTTPData.version;
    switch (method)
    {
        case "GET":
            HTTPData.method = GET;
            break;
        case "POST":
            HTTPData.method = POST;
            break;
        case "DELETE":
            HTTPData.method = DELETE;
            break;
        default:
            HTTPData.method = UNKNOWN;
            break;
    }
    return HTTPData;
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