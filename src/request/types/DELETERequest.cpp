#include "DELETERequest.hpp"

DELETERequest::DELETERequest(HTTPRequestData data, const LocationConfig* location_config) :
    HTTPRequest(data, location_config)
{}

std::string DELETERequest::handle()
{
    // Check method allowed
    if (!_effective_config->getLimitExcept().empty() &&
        _effective_config->getLimitExcept().find("delete") == _effective_config->getLimitExcept().end())
    {
        return errorResponse(405);
    }

    // Redirection handling
    if (_effective_config->getReturn().first != -1)
        return handleRedirection(_effective_config->getReturn());

    // Resolve target resource path
    std::filesystem::path targetPath{_effective_config->getRoot()};
    targetPath /= getURInoLeadingSlash();

    // Prevent escaping root
    std::filesystem::path safePath;
    if (!normalizeAndValidateUnderRoot(targetPath, safePath))
        return errorResponse(403);

    if (!std::filesystem::exists(safePath))
        return errorResponse(404);

    if (std::filesystem::is_directory(safePath))
        return errorResponse(403);

    // Remove the file
    std::error_code ec;
    bool removed = std::filesystem::remove(safePath, ec);
    if (ec)
        return errorResponse(500);

    if (!removed)
        return errorResponse(404);

    ResponseWriter response(200, {{"Content-Type", "text/plain"}}, std::string("Deleted \"") + safePath.filename().string() + "\"\n");
    return response.write();
}