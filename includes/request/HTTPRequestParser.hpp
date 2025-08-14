#pragma once

#include "HTTPRequestData.hpp"
#include "utils.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm> /* std::tranform() */

class HTTPRequestData;
class HTTPRequestParser
{
private:
    static std::unordered_map<std::string, std::string> parseHeaders(std::istringstream headerStream);
    static HTTPRequestData getRequestLine(std::istringstream &headerStream);
    static std::string getBody (const std::unordered_map<std::string, std::string>& headers, const std::string &requestStr);
    static std::string parseChunkedBody(const std::string &bodyStr);
public:
    static bool isValidRequest(const std::string &request_str);
    static HTTPRequestData parse(const std::string &request_str);
};