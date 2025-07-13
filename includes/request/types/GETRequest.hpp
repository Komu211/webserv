#pragma once

#include "../HTTPRequest.hpp"

class GETRequest final : public HTTPRequest
{
public:
    void handle() override;
};