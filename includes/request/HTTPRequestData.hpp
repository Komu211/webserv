#pragma once

#include <string>
#include <unordered_map>

enum HTTPMethod { GET, POST, DELETE, NONE };

struct HTTPRequestData
{
    const HTTPMethod method;
    const std::string uri;
    const std::string version;
    std::unordered_map<std::string, std::string> headers;
    const std::string body;
};