#pragma once

#include "../HTTPRequest.hpp"

class POSTRequest final : public HTTPRequest
{
public:
    POSTRequest() = delete;
    explicit POSTRequest(HTTPRequestData data);
    POSTRequest(const POSTRequest &) = default;
    POSTRequest(POSTRequest &&) = default;
    ~POSTRequest() override = default;
    
    std::string handle() override;
};