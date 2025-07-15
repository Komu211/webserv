#pragma once

#include "../HTTPRequest.hpp"

class ErrorRequest final : public HTTPRequest
{
private:
    int _errorCode;
public:
    ErrorRequest() = delete;
    explicit ErrorRequest(HTTPRequestData data, int errorCode);
    ErrorRequest(const ErrorRequest &) = default;
    ErrorRequest(ErrorRequest &&) = default;
    ~ErrorRequest() override = default;
    
    std::string handle() override;
};