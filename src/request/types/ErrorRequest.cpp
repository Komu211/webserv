#include "ErrorRequest.hpp"

ErrorRequest::ErrorRequest(HTTPRequestData data, int errorCode, const LocationConfig* location_config) :
    HTTPRequest(data, location_config), _errorCode(errorCode)
{}

std::string ErrorRequest::getFullResponse()
{
    ResponseWriter response(_errorCode, {{"Content-Type", "text/html"}}, getErrorResponseBody(_errorCode));
    return response.write();
}