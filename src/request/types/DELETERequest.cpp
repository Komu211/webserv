#include "../../../includes/request/types/DELETERequest.hpp"

DELETERequest::DELETERequest(HTTPRequestData data) :
    HTTPRequest(data)
{}

void DELETERequest::handle()
{
    std::cout << "DELETE Request: " << _data.body << std::endl;
}