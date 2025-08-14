#pragma once

#include "HTTPRequest.hpp"

class DELETERequest final : public HTTPRequest
{
public:
    DELETERequest() = delete;
    explicit DELETERequest(HTTPRequestData data, const LocationConfig* location_config);
    DELETERequest(const DELETERequest &) = delete;
    DELETERequest(DELETERequest &&) = delete;
    ~DELETERequest() override = default;

    void generateResponse(Server* server, int clientFd) override;

    virtual void continuePrevious() override;
};
