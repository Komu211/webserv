#pragma once

#include "HTTPRequest.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <chrono>

class POSTRequest final : public HTTPRequest
{
public:
    POSTRequest() = delete;
    explicit POSTRequest(HTTPRequestData data, const LocationConfig* location_config);
    POSTRequest(const POSTRequest &) = delete;
    POSTRequest(POSTRequest &&) = delete;
    ~POSTRequest() override = default;

    struct MultipartPart {
        std::string contentDisposition;
        std::string contentType;
        std::string name;
        std::string filename;
        std::string content;
    };
    
    void handleMultipart();
    void handleUrlEncoded();
    void handleFileUpload(const std::vector<MultipartPart*>& fileParts);
    void handleFormFields(const std::vector<MultipartPart> &parts);
    std::vector<MultipartPart> parseMultipartFormData();
    MultipartPart parsePartContent(const std::string& partContent);
    std::vector<MultipartPart*> findAllUploadFileParts(const std::vector<MultipartPart> &parts);


    void generateResponse(Server* server, int clientFd) override;

    virtual void continuePrevious() override;
};
