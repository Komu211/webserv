#pragma once

#include "HTTPRequest.hpp"

class GETRequest final : public HTTPRequest
{
public:
    GETRequest() = delete;
    explicit GETRequest(HTTPRequestData data, const LocationConfig *location_config);
    GETRequest(const GETRequest &) = delete;
    GETRequest(GETRequest &&) = delete;
    ~GETRequest() override = default;

    void generateResponse(Server *server, int clientFd) override;

private:
    void serveFile(const std::filesystem::path &filePath);

    virtual void continuePrevious() override;
};
