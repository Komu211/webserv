#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "HTTPRequestData.hpp"
#include "LocationConfig.hpp"

class LocationConfig;
class HTTPRequest
{
protected:
    HTTPRequestData _data;
    const LocationConfig* _effective_config;

public:
    HTTPRequest() = delete;
    explicit HTTPRequest(HTTPRequestData data, const LocationConfig* location_config);
    virtual ~HTTPRequest() = default;

    // Where the magic happens
    virtual std::string handle() = 0;

    [[nodiscard]] bool isCloseConnection() const;
};
