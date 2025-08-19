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

std::vector<POSTRequest::MultipartPart> POSTRequest::parseMultipartFormData()
{
    std::vector<MultipartPart> parts;

    // Extract boundary from Content-Type header
    auto contentTypeIt = _data.headers.find("content-type");
    if (contentTypeIt == _data.headers.end()) {
        return parts;
    }
    
    std::string contentType = contentTypeIt->second;
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos) {
        return parts;
    }
    
    std::string boundary = contentType.substr(boundaryPos + 9);
    // Remove quotes
    if (boundary.front() == '"' && boundary.back() == '"') {
        boundary = boundary.substr(1, boundary.length() - 2);
    }
    
    if (boundary.empty()) {
        return parts;
    }

    
    std::string boundaryDelimiter = "--" + boundary;
    std::string boundaryEnd = boundaryDelimiter + "--";

    
    std::size_t pos = 0;
    const std::string& body = _data.body;
    
    // Find first boundary
    pos = body.find(boundaryDelimiter, pos);
    if (pos == std::string::npos) {
        return parts; // No boundary found
    }
    
    while (pos != std::string::npos) {
        pos += boundaryDelimiter.length();
        
        // Skip CRLF after boundary
        if (pos < body.size() && body[pos] == '\r') pos++;
        if (pos < body.size() && body[pos] == '\n') pos++;
        
        // Find next boundary
        std::size_t endPos = body.find(boundaryDelimiter, pos);
        if (endPos == std::string::npos) {
            // Check for end boundary
            endPos = body.find(boundaryEnd, pos);
            if (endPos == std::string::npos) {
                break; // No more parts
            }
        }
        
        // Extract part content (exclude trailing CRLF before boundary)
        std::string partContent = body.substr(pos, endPos - pos);
        if (partContent.length() >= 2 && partContent.substr(partContent.length() - 2) == "\r\n") {
            partContent = partContent.substr(0, partContent.length() - 2);
        }
        
        // Parse this part
        MultipartPart part = parsePartContent(partContent);
        if (!part.contentDisposition.empty()) {
            parts.push_back(part);
        }
        
        pos = endPos;
        // Check if this is the end boundary
        if (pos < body.size() && body.substr(pos, boundaryEnd.length()) == boundaryEnd) {
            break;
        }
    }
    
    
    return parts;
}

POSTRequest::MultipartPart POSTRequest::parsePartContent(const std::string& partContent)
{
    MultipartPart part;

    // 1. Split headers and content
    std::size_t headerEnd = partContent.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return part; // Invalid part

    std::string headers = partContent.substr(0, headerEnd);
    part.content = partContent.substr(headerEnd + 4); // skip "\r\n\r\n"

    // 2. Parse headers line by line
    std::istringstream stream(headers);
    std::string line;

    while (std::getline(stream, line)) {
        // Remove trailing CR if present
        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (line.find("Content-Disposition:") == 0) {
            part.contentDisposition = line;

            // Try to extract name and filename
            std::size_t namePos = line.find("name=\"");
            if (namePos != std::string::npos) {
                std::size_t start = namePos + 6;
                std::size_t end = line.find("\"", start);
                if (end != std::string::npos)
                    part.name = line.substr(start, end - start);
            }

            std::size_t filenamePos = line.find("filename=\"");
            if (filenamePos != std::string::npos) {
                std::size_t start = filenamePos + 10;
                std::size_t end = line.find("\"", start);
                if (end != std::string::npos)
                    part.filename = line.substr(start, end - start);
            }

        } else if (line.find("Content-Type:") == 0) {
            part.contentType = line.substr(13);
            // Trim leading whitespace
            while (!part.contentType.empty() && std::isspace(part.contentType[0]))
                part.contentType.erase(0, 1);
        }
    }

    return part;
}


void POSTRequest::extractFileInfo(MultipartPart& part) {
    
    if (!part.filename.empty()) {
        _filename = part.filename;
        std::cout << "Extracted filename: " << _filename << std::endl;
    }
}

void POSTRequest::handleFileUpload() {
    // Reused old logic


    // Check if upload store is configured
    if (_effective_config->getUploadStore().empty())
        return errorResponse(501); 

    // Resolve upload directory
    std::filesystem::path uploadDir{_effective_config->getUploadStore()};
    if (uploadDir.is_relative())
        uploadDir = std::filesystem::path(_effective_config->getRoot()) / uploadDir;

    // Prevent escaping root
    std::filesystem::path safeUploadDir;
    if (!normalizeAndValidateUnderRoot(uploadDir, safeUploadDir))
        return errorResponse(403);

    // Ensure directory exists
    std::error_code ec;
    if (!std::filesystem::exists(safeUploadDir))
    {
        std::filesystem::create_directories(safeUploadDir, ec);
        if (ec)
            return errorResponse(500);
    }


    // TODO: CHANGE TO ALLOW MULTIPLE FILES ?!

    
    // Use extracted filename or generate one
    std::string filename = _filename;
    if (filename.empty()) {
        // Generate filename with timestamp
        auto now = std::chrono::system_clock::now();
        auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        filename = std::string("upload_") + std::to_string(epoch) + ".bin";
    }

    std::filesystem::path targetPath = safeUploadDir / filename;

    // If file exists, append number to avoid overwrite
    if (std::filesystem::exists(targetPath))
    {
        std::string stem = targetPath.stem().string();
        std::string ext = targetPath.extension().string();
        for (int i = 1; i < 10000; ++i)
        {
            std::filesystem::path candidate = safeUploadDir / (stem + "_" + std::to_string(i) + ext);
            if (!std::filesystem::exists(candidate))
            {
                targetPath = candidate;
                break;
            }
        }
    }

    // Open file for writing
    int fd = open(targetPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        std::cerr << "Failed to open file for writing: " << strerror(errno) << std::endl;
        return errorResponse(500);
    }

    // Set to non-blocking mode
    try {
        setNonBlocking(fd);
    } catch (const std::exception& e) {
        close(fd);
        std::cerr << "Failed to set file to non-blocking: " << e.what() << std::endl;
        return errorResponse(500);
    }

    // Find file content from the parsed multipart data
    // TODO: One Upload or Multiple Uploads?
    std::vector<MultipartPart> parts = parseMultipartFormData();
    MultipartPart* filePart = findUploadFilePart(parts);
    
    if (!filePart) {
        close(fd);
        return errorResponse(400);
    }

    // Create OpenFile structure for writing
    OpenFile openFile;
    openFile.fileType = OpenFile::WRITE;
    openFile.content = filePart->content;
    openFile.size = filePart->content.size();
    openFile.finished = false;

    // Register file with server
    _clientData->openFiles[fd] = openFile;
    _server->getOpenFilesToClientMap()[fd] = _clientFd;
    _server->getPollManager().addWriteFileFd(fd);

    std::cout << "File upload initiated for: " << targetPath.filename() << " (" << openFile.size << " bytes)" << std::endl;
}

void POSTRequest::handleFormFields(const std::vector<MultipartPart>& parts) {
    // TODO: Implement logic
    (void)parts;
}

void POSTRequest::handleUrlEncoded() {
    // TODO: Implement logic
}

POSTRequest::MultipartPart* POSTRequest::findUploadFilePart(const std::vector<MultipartPart>& parts) {
    // TODO: We only accept one file or accept multiple? Current approach is one file but Struct is designed for multiple.
    for (auto& part : const_cast<std::vector<MultipartPart>&>(parts)) {
        if (!part.filename.empty()) {
            return &part;
        }
    }
    return nullptr;
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
                ResponseWriter response(201, {{"Content-Type", "text/plain"}}, "File uploaded successfully\n");
                _fullResponse = response.write();
            }
        }
    }
    if (num_ready == _clientData->openFiles.size())
        _responseState = READY;
}

