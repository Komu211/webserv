#pragma once

#include "../HTTPRequest.hpp"

class POSTRequest final : public HTTPRequest
{
public:
    void handle() override;
};