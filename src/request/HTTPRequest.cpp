#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest(HTTPRequestData data):
    _data(std::move(data))
{}