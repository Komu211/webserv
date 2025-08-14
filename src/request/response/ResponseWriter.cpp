#include "ResponseWriter.hpp"

ResponseWriter::ResponseWriter(int statusCode, const std::unordered_map<std::string, std::string> &headers, const std::string &response_body)
    : _start_line(std::string("HTTP/1.1 ") + std::to_string(statusCode) + " " + reasonPhraseFromStatusCode(statusCode))
    , _headers()
    , _response_body(response_body)
{
    _headers["Date"] = getCurrentGMTString();
    _headers["Server"] = "Webserv";
    _headers["Content-Length"] = std::to_string(response_body.length());

    for (const auto &[key, value] : headers)
        _headers[key] = value;
}

void ResponseWriter::setBody(const std::string& body)
{
    _response_body = body;
}

std::string ResponseWriter::write()
{
    _headers["Content-Length"] = std::to_string(_response_body.length());

    std::string responseStr {_start_line + "\r\n"};
    responseStr.reserve(200 + _response_body.length());

    for (const auto &[key, value] : _headers)
    {
        responseStr.append(key + ": ");
        responseStr.append(value + "\r\n");
    }

    responseStr.append("\r\n");

    responseStr.append(_response_body);

    return responseStr;
}
