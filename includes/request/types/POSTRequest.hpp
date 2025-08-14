#pragma once

#include "HTTPRequest.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <chrono>

class POSTRequest final : public HTTPRequest
{
public:
    POSTRequest() = delete;
    explicit POSTRequest(HTTPRequestData data, const LocationConfig* location_config);
    POSTRequest(const POSTRequest &) = delete;
    POSTRequest(POSTRequest &&) = delete;
    ~POSTRequest() override = default;
    
    void generateResponse(Server* server, int clientFd) override;

    virtual void continuePrevious() override;
};
