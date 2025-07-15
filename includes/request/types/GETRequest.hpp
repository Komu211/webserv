#pragma once

#include "../HTTPRequest.hpp"

class GETRequest final : public HTTPRequest
{
public:
    GETRequest() = delete;
    explicit GETRequest(HTTPRequestData data);
    GETRequest(const GETRequest &) = default;
    GETRequest(GETRequest &&) = default;
    ~GETRequest() override = default;
    
    std::string handle() override;
};