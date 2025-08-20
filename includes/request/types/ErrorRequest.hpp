#pragma once

#include "HTTPRequest.hpp"

class ErrorRequest final : public HTTPRequest
{
private:
    int _errorCode;
public:
    ErrorRequest() = delete;
    explicit ErrorRequest(HTTPRequestData data, int errorCode, const LocationConfig* location_config);
    ErrorRequest(const ErrorRequest &) = delete;
    ErrorRequest(ErrorRequest &&) = delete;
    ~ErrorRequest() override = default;
    
    void generateResponse(Server* server, int clientFd) override;

    virtual void continuePrevious() override;
};
