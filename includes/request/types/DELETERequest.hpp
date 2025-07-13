#pragma once

#include "../HTTPRequest.hpp"

class DELETERequest final : public HTTPRequest
{
public:
    void handle() override;
};