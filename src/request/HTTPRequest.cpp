#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest(HTTPRequestData data)
    : _data(std::move(data))
{
}

bool HTTPRequest::isCloseConnection() const
{
    return _data.headers.find("Connection") != _data.headers.end() &&
           _data.headers.at("Connection") == "close";
}