#include "ErrorRequest.hpp"
#include "Server.hpp"

ErrorRequest::ErrorRequest(HTTPRequestData data, int errorCode, const LocationConfig *location_config)
    : HTTPRequest(data, location_config)
    , _errorCode(errorCode)
{
}

void ErrorRequest::generateResponse(Server *server, int clientFd)
{
    _server = server;
    _clientFd = clientFd;
    _clientData = &(_server->getClientDataMap()[_clientFd]);

    // Check if we are generating brand new response or continuing previous
    if (_responseState != NOT_STARTED)
        return continuePrevious();

    _responseState = IN_PROGRESS;

    errorResponse(_errorCode);
}

void ErrorRequest::continuePrevious()
{
    std::size_t num_ready{0};
    for (auto &[fileFd, fileData] : _clientData->openFiles)
    {
        if (fileData.finished)
        {
            if (fileData.fileType == OpenFile::READ)
            {
                ++num_ready;
                if (_responseWithoutBody)
                {
                    // no CGI here
                    _responseWithoutBody->setBody(fileData.content);
                    _fullResponse = _responseWithoutBody->write();
                }
            }
            else if (fileData.fileType == OpenFile::WRITE)
            {
                ++num_ready;
                // nothing to do. never writes anyway
            }
        }
    }
    if (num_ready == _clientData->openFiles.size())
        _responseState = READY;
}
