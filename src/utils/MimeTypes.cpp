#include "MimeTypes.hpp"

const std::unordered_map<std::string_view, std::string_view> MimeTypes::_mimeToExt{generateMimeToExt()};
const std::unordered_map<std::string_view, std::string_view> MimeTypes::_extToMime{generateExtToMime()};

/* Public member functions */

std::string_view MimeTypes::getExtension(std::string_view mime)
{
    auto it{_mimeToExt.find(mime)};
    if (it == _mimeToExt.end())
        return "";
    return it->second;
}

std::string_view MimeTypes::getMimeType(std::string_view ext)
{
    auto it{_extToMime.find(ext)};
    if (it == _extToMime.end())
        return "application/octet-stream"; // for unknown extensions
    return it->second;
}

/* Private generators run once to create database */

std::unordered_map<std::string_view, std::string_view> MimeTypes::generateMimeToExt()
{
    std::unordered_map<std::string_view, std::string_view> map;
    for (const auto &[mimeType, extension] : _mime_data)
        map[mimeType] = extension;
    return map;
}

std::unordered_map<std::string_view, std::string_view> MimeTypes::generateExtToMime()
{
    std::unordered_map<std::string_view, std::string_view> map;
    for (const auto &[mimeType, extension] : _mime_data)
        map[extension] = mimeType;
    return map;
}
