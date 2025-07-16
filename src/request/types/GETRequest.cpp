#include "../../../includes/request/types/GETRequest.hpp"

GETRequest::GETRequest(HTTPRequestData data) :
    HTTPRequest(data)
{}

std::string GETRequest::handle()
{
    // Replace this with actual logic for handling GET requests
    return "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\ncontent-length: 13\r\n\r\nHello, World!";
}