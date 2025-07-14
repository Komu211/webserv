#pragma once

#include "HTTPRequestData.hpp"
#include "utils.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

class HTTPRequestParser
{
private:
    static std::unordered_map<std::string, std::string> parseHeaders(std::istringstream headerStream);
public:
    static bool isValidRequest(const std::string &request_str);
    static HTTPRequestData parse(const std::string &request_str);
};