#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest(HTTPRequestData data, const LocationConfig* location_config)
    : _data(std::move(data)), _effective_config(location_config)
{
}

bool HTTPRequest::isCloseConnection() const
{
    return _data.headers.find("Connection") != _data.headers.end() &&
           _data.headers.at("Connection") == "close";
}