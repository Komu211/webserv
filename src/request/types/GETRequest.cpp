#include "../../../includes/request/types/GETRequest.hpp"

GETRequest::GETRequest(HTTPRequestData data) :
    HTTPRequest(data)
{}

void GETRequest::handle()
{
    std::cout << "GET Request: " << _data.body << std::endl;
}