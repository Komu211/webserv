#include "HTTPRequestFactory.hpp"

std::unique_ptr<HTTPRequest> HTTPRequestFactory::createRequest(const HTTPRequestData &data)
{
    switch (data.method)
    {
    case GET:
        return std::make_unique<GETRequest>(data);
    case POST:
        return std::make_unique<POSTRequest>(data);
    case DELETE:
        return std::make_unique<DELETERequest>(data);
    case BAD_REQUEST:
        return std::make_unique<ErrorRequest>(data, 400);
    default:
        return std::make_unique<ErrorRequest>(data, 501);
    }
}