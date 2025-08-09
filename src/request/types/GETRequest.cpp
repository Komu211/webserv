#include "GETRequest.hpp"

GETRequest::GETRequest(HTTPRequestData data, const LocationConfig* location_config) :
    HTTPRequest(data, location_config)
{}

std::string GETRequest::handle()
{
    // Create response body string (either file contents, directory listing, or error)
    // Create std::unordered_map<std::string, std::string> with headers such as "Content-Type" and "Last-Modified"
    // Determine response code
    // Pass the above three to create ResponseWriter object
    // Return responseWriter.write()

    // Replace this with actual logic for handling GET requests
    return "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\ncontent-length: 22\r\n\r\nHello, World from GET!";
}