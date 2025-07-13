#include "../../includes/request/HTTPRequestParser.hpp"

bool HTTPRequestParser::is_valid_request(const std::string &request_str)
{
    // TODO: Implement validation logic
    return true;
}

HTTPRequestData HTTPRequestParser::parse(const std::string &request_str)
{
    // TODO: Implement parsing logic
    HTTPRequestData data = {
        GET,
        "/",
        "1.1",
        {},
        ""
    };
    return data;
}