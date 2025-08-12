#include "HTTPRequestData.hpp"

std::string HTTPRequestData::methodStr() const
{
    switch (method)
    {
    case GET:
        return "GET";
    case POST:
        return "POST";
    case DELETE:
        return "DELETE";
    case NONE:
        return "NONE";
    case UNKNOWN:
        return "UNKNOWN";
    case BAD_REQUEST:
        return "BAD_REQUEST";
    default:
        return "UNKNOWN";
    }
}