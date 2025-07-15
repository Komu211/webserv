#pragma once

#include "../HTTPRequest.hpp"

class DELETERequest final : public HTTPRequest
{
public:
    DELETERequest() = delete;
    explicit DELETERequest(HTTPRequestData data);
    DELETERequest(const DELETERequest &) = default;
    DELETERequest(DELETERequest &&) = default;
    ~DELETERequest() override = default;

    std::string handle() override;
};