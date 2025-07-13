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
    POSTRequest& operator=(const POSTRequest &) = default;
    POSTRequest& operator=(POSTRequest &&) = default;
    
    void handle() override;
};