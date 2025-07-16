#include "../../../includes/request/types/ErrorRequest.hpp"

ErrorRequest::ErrorRequest(HTTPRequestData data, int errorCode) :
    HTTPRequest(data), _errorCode(errorCode)
{}

std::string ErrorRequest::handle()
{
    //Better handling needed, enough for now
    switch (_errorCode)
    {
    case 400:
        return "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 11\r\n\r\nBad Request";
    case 404:
        return "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot Found";
    case 405:
        // Added as a useful case
        return "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/plain\r\nContent-Length: 18\r\n\r\nMethod Not Allowed";
    case 501:
        return "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/plain\r\nContent-Length: 15\r\n\r\nNot Implemented";
    default: // Fallback to 500
        return "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\nContent-Length: 21\r\n\r\nInternal Server Error";
    }
}