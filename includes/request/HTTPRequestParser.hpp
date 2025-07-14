#pragma once

#include "HTTPRequestData.hpp"

class HTTPRequestParser
{
public:
    static bool isValidRequest(const std::string &request_str);
    static HTTPRequestData parse(const std::string &request_str);
};