#include "POSTRequest.hpp"
#include "Server.hpp"

POSTRequest::POSTRequest(HTTPRequestData data, const LocationConfig* location_config) :
    HTTPRequest(data, location_config)
{}

void POSTRequest::generateResponse(Server* server, int clientFd)
{
    _server = server;
    _clientFd = clientFd;
    _clientData = &(_server->getClientDataMap()[_clientFd]);

    // Check if we are generating brand new response or continuing previous
    if (_responseState != NOT_STARTED)
        return continuePrevious();

    _responseState = IN_PROGRESS;

    // Check POST method allowed
    if (!_effective_config->getLimitExcept().empty() &&
        _effective_config->getLimitExcept().find("post") == _effective_config->getLimitExcept().end())
    {
        return errorResponse(405);
    }

    if (_effective_config->getReturn().first != -1)
        return handleRedirection(_effective_config->getReturn());

    // Check client_max_body_size
    const std::size_t maxAllowedBody = _effective_config->getClientMaxBodySize();
    if (_data.body.size() > maxAllowedBody)
        return errorResponse(413);

    // Check if this is a CGI request first
    std::filesystem::path cgiTargetPath{_effective_config->getRoot()};
    cgiTargetPath /= getURInoLeadingSlash();
    
    // Normalize and validate the path
    std::filesystem::path safePath;
    if (!normalizeAndValidateUnderRoot(cgiTargetPath, safePath))
        return errorResponse(403);
    
    // If path exists and is a directory, check for index files
    std::filesystem::path finalPath = safePath;
    if (std::filesystem::exists(safePath) && std::filesystem::is_directory(safePath))
    {
        for (const auto &file : _effective_config->getIndexFilesVec())
        {
            std::filesystem::path indexPath = safePath / file;
            if (std::filesystem::exists(indexPath))
            {
                finalPath = indexPath;
                break;
            }
        }
    }
    
    // Check if the final resolved path matches any CGI handler
    for (const auto &[extension, interpreter] : _effective_config->getCGIHandlersMap())
    {
        if (finalPath.extension().string() == extension)
        {
            std::cout << "CGI detected for: " << finalPath << std::endl;
            return serveCGI(finalPath, interpreter);
        }
    }

    std::cout << "Not a CGI request, handling as file upload" << std::endl;
    
    auto foundHeaders = _data.headers.find("content-type");
    if (foundHeaders == _data.headers.end())
        return errorResponse(400); // Missing Content-Type header
    
    std::string contentType = foundHeaders->second;
    std::transform(contentType.begin(), contentType.end(), contentType.begin(), ::tolower);
    
    if (contentType.find("multipart/form-data") != std::string::npos)
        return handleMultipart();
    else if (contentType == "application/x-www-form-urlencoded")
        return handleUrlEncoded();
    else
        return errorResponse(415); // Unsupported Media Type    
}

void POSTRequest::handleMultipart()
{
    std::cout << "Handling multipart form data" << std::endl;
    if (!_filename.empty()) {
        return handleFileUpload();
    }

    std::vector<MultipartPart> parts = parseMultipartFormData();
    if (parts.empty())
    {
        std::cout << "Failed to parse multipart form data -> nothing valid found" << std::endl;
        return errorResponse(400);
    }

    MultipartPart* part = findUploadFilePart(parts);
    if (part)
    {
        extractFileInfo(*part);
        return handleFileUpload();
    }
    else {
        handleFormFields(parts);
        _responseState = READY;
    }
}

void POSTRequest::continuePrevious()
{
    std::size_t num_ready{0};
    for (auto& [fileFd, fileData] : _clientData->openFiles)
    {
        if (fileData.finished)
        {
            if (fileData.fileType == OpenFile::READ)
            {
                ++num_ready;
                if (_responseWithoutBody)
                {
                    // TODO: if is CGI response, convert to full response
                    _responseWithoutBody->setBody(fileData.content);
                    _fullResponse = _responseWithoutBody->write();
                }
            }
            else if (fileData.fileType == OpenFile::WRITE)
            {
                ++num_ready;
                // TODO: Write to file or CGI
            }
        }
    }
    if (num_ready == _clientData->openFiles.size())
        _responseState = READY;
}

