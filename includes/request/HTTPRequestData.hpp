#pragma once

#include <string>
#include <unordered_map>

enum HTTPMethod { GET, POST, DELETE, NONE };

struct HTTPRequestData
{
    HTTPMethod method;
    std::string uri;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};