#include "../../../includes/request/types/POSTRequest.hpp"

POSTRequest::POSTRequest(HTTPRequestData data) :
    HTTPRequest(data)
{}

std::string POSTRequest::handle()
{
    // Replace this with actual logic for handling GET requests
    return "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\ncontent-length: 13\r\n\r\nHello, World!";
}
