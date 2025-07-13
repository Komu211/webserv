#pragma once

#include <string>
#include <vector>
#include "HTTPRequestData.hpp"

class HTTPRequest
{
private:
    HTTPRequestData _data;

public:
    virtual ~HTTPRequest() = default;

    virtual void handle() = 0;
};
