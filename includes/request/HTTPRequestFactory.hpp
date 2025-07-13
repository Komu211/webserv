#pragma once

#include "HTTPRequest.hpp"
#include <memory>

class HTTPRequestFactory
{
public:
    static std::unique_ptr<HTTPRequest> createRequest(const HTTPRequestData& data);
};