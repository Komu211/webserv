#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "HTTPRequestData.hpp"

class HTTPRequest
{
protected:
    HTTPRequestData _data;

public:
    HTTPRequest() = delete;
    explicit HTTPRequest(HTTPRequestData data);
    virtual ~HTTPRequest() = default;

    // Where the magic happens
    virtual std::string handle() = 0;

    [[nodiscard]] bool isCloseConnection() const;
};
