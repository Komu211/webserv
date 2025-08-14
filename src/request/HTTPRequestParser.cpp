#include "HTTPRequestParser.hpp"

#include <iostream>

bool HTTPRequestParser::isValidRequest(const std::string &buffer)
{
    auto bodyStart = buffer.find("\r\n\r\n");
    if (bodyStart == std::string::npos)
        return false;
    std::istringstream headerStream(buffer.substr(0, bodyStart));
    bodyStart += 4; // Skip CRLF CRLF
    auto headers = parseHeaders(std::move(headerStream));
    if (headers.find("content-length") != headers.end())
    {
        auto contentLength = std::stoul(headers["content-length"]);
        if (bodyStart + contentLength > buffer.size())
            return false;
    }
    if (headers.find("transfer-encoding") != headers.end()
        && headers["transfer-encoding"] == "chunked")
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
    try
    {
        HTTPData.body = getBody(HTTPData.headers, requestStr.substr(bodyStart));
    }
    catch (const std::exception &e)
    {
        std::cout << "[info] Failed to parse body: " << e.what() << std::endl;
        return {BAD_REQUEST, "", "", {}, ""};
    }
    return HTTPData;
}

std::size_t HTTPRequestParser::getResponseSizeFromCgiHeader(const std::string& cgiResponseStr)
{
    std::size_t size{0};
    auto bodyStart = cgiResponseStr.find("\r\n\r\n");
    if (bodyStart != std::string::npos)
        size += 4;
    std::istringstream headerStream(cgiResponseStr.substr(0, bodyStart));
    size += headerStream.str().length();
    [[maybe_unused]]auto HTTPData = getRequestLine(headerStream);
    HTTPData.headers = parseHeaders(std::move(headerStream));
    auto content_length_header {HTTPData.headers.find("content-length")};
    if (content_length_header == HTTPData.headers.end())
        return std::string::npos;
    size += std::stoul(content_length_header->second);
    return size;
}

std::string HTTPRequestParser::getBody(const std::unordered_map<std::string, std::string> &headers, const std::string &bodyStr)
{
    auto transferEncodingIt = headers.find("transfer-encoding");
    if (transferEncodingIt != headers.end() && transferEncodingIt->second == "chunked")
    {
        return parseChunkedBody(bodyStr);
    }
    auto contentLengthIt = headers.find("content-length");
    if (contentLengthIt != headers.end())
    {
        size_t bodyLength = std::stoul(contentLengthIt->second);
        if (bodyStr.size() < bodyLength)
            throw std::runtime_error("Invalid content length.");
        return bodyStr.substr(0, bodyLength);
    }
    return "";
}

std::string HTTPRequestParser::parseChunkedBody(const std::string &bodyStr)
{
    std::stringstream bodyStream(bodyStr);
    std::string result;
    std::string line;

    while (bodyStream.good())
    {
        std::getline(bodyStream, line);
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        // 2. Parse the size from hexadecimal.
        size_t chunkSize;
        try
        {
            chunkSize = std::stoul(line, nullptr, 16);
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error("Invalid chunk size format.");
        }

        // 3. If chunk size is 0, this is the last chunk.
        if (chunkSize == 0)
        {
            while (std::getline(bodyStream, line) && !line.empty() && line != "\r")
                ; // Get rid of trailing headers.
            break;
        }

        // 4. Read exactly 'chunkSize' bytes of data.
        std::vector<char> buffer(chunkSize);
        bodyStream.read(buffer.data(), chunkSize);

        if (static_cast<size_t>(bodyStream.gcount()) != chunkSize)
        {
            throw std::runtime_error("Malformed chunk: unexpected end of data.");
        }
        result.append(buffer.data(), chunkSize);
        bodyStream.ignore(2);
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
    if (method == "GET")
        HTTPData.method = GET;
    else if (method == "POST")
        HTTPData.method = POST;
    else if (method == "DELETE")
        HTTPData.method = DELETE;
    else
        HTTPData.method = UNKNOWN;
    trim(HTTPData.uri);
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
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            trim(value);
            headers[key] = value;
        }
    }
    return headers;
}