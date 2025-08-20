#include "HTTPRequestFactory.hpp"

std::unique_ptr<HTTPRequest> HTTPRequestFactory::createRequest(const HTTPRequestData &data, const LocationConfig* location_config)
{
    switch (data.method)
    {
    case GET:
        return std::make_unique<GETRequest>(data, location_config);
    case POST:
        return std::make_unique<POSTRequest>(data, location_config);
    case DELETE:
        return std::make_unique<DELETERequest>(data, location_config);
    case BAD_REQUEST:
        return std::make_unique<ErrorRequest>(data, 400, location_config);
    default:
        return std::make_unique<ErrorRequest>(data, 501, location_config);
    }
}