#pragma once

#include <memory>
#include "HTTPRequest.hpp"
#include "HTTPRequestData.hpp"
#include "./types/GETRequest.hpp"
#include "./types/POSTRequest.hpp"
#include "./types/DELETERequest.hpp"

class HTTPRequestFactory
{
public:
    static std::unique_ptr<HTTPRequest> createRequest(const HTTPRequestData& data);
};