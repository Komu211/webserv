#pragma once

#include "HTTPRequest.hpp"

class GETRequest final : public HTTPRequest
{
public:
    GETRequest() = delete;
    explicit GETRequest(HTTPRequestData data, const LocationConfig *location_config);
    GETRequest(const GETRequest &) = default;
    GETRequest(GETRequest &&) = default;
    ~GETRequest() override = default;

    std::string handle() override;

private:
    [[nodiscard]] std::string serveFile(const std::filesystem::path &filePath) const;
};
