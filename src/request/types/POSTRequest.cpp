#include "../../../includes/request/types/POSTRequest.hpp"

POSTRequest::POSTRequest(HTTPRequestData data) :
    HTTPRequest(data)
{}

void POSTRequest::handle()
{
    std::cout << "POST Request: " << _data.body << std::endl;
}
