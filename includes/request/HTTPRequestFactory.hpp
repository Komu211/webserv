#pragma once

#include <memory>
#include "HTTPRequest.hpp"
#include "HTTPRequestData.hpp"
#include "GETRequest.hpp"
#include "POSTRequest.hpp"
#include "DELETERequest.hpp"
#include "ErrorRequest.hpp"

class HTTPRequestFactory
{
public:
    static std::unique_ptr<HTTPRequest> createRequest(const HTTPRequestData& data);
};