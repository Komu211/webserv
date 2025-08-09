#include "GETRequest.hpp"

GETRequest::GETRequest(HTTPRequestData data, const LocationConfig* location_config) :
    HTTPRequest(data, location_config)
{}

std::string GETRequest::handle()
{
    // Replace this with actual logic for handling GET requests
    return "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\ncontent-length: 22\r\n\r\nHello, World from GET!";
}