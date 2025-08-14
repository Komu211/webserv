#include "POSTRequest.hpp"

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

    // Determine filename (use last path segment)
    // TODO: sanitize filename
    std::string uriPathStr = getURInoLeadingSlash();
    std::string filenameCandidate = std::filesystem::path(uriPathStr).filename().string();
    if (filenameCandidate.empty() || filenameCandidate == "." || filenameCandidate == "..")
    {
        // generate filename with timestamp
        auto now = std::chrono::system_clock::now();
        auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        filenameCandidate = std::string("upload_") + std::to_string(epoch) + ".bin";
    }

    std::filesystem::path targetPath = safeUploadDir / filenameCandidate;

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

    // Stream body to disk in chunks
    // TODO: add to poll manager, set state, and come back to write later
    try
    {
        std::ofstream out(targetPath, std::ios::binary);
        if (!out.is_open())
            return errorResponse(500);

        const std::string &bodyRef = _data.body;
        const std::size_t chunkSize = 1 << 15; // 32KB
        for (std::size_t offset = 0; offset < bodyRef.size(); offset += chunkSize)
        {
            std::size_t toWrite = std::min(chunkSize, bodyRef.size() - offset);
            out.write(bodyRef.data() + static_cast<std::streamsize>(offset), static_cast<std::streamsize>(toWrite));
            if (!out)
                return errorResponse(500);
        }
        out.close();
    }
    catch (const std::exception &)
    {
        return errorResponse(500);
    }

    // Success response
    // TODO: move to continuePrevious()
    std::ostringstream body;
    body << "Uploaded '" << targetPath.filename().string() << "' (" << _data.body.size() << ") bytes\n";
    ResponseWriter response(201, {{"Content-Type", "text/plain"}}, body.str());
    _fullResponse = response.write();
    _responseState = READY;
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

