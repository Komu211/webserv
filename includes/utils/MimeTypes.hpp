#pragma once

#include <string>
#include <array>
#include <unordered_map>

class MimeTypes
{
public:
    // Get extension (e.g., .html) from a given MIME type (e.g., "text/html")
    static std::string_view getExtension(std::string_view mime);
    // Get MIME type (e.g., "text/html") from a given extension (e.g., .html)
    static std::string_view getMimeType(std::string_view ext);

    // OCF
    MimeTypes() = default;
    MimeTypes(const MimeTypes& other) = default;
    MimeTypes& operator=(const MimeTypes& other) = default;
    ~MimeTypes() = default;

private:
    // Single source of truth
    static constexpr std::array<std::pair<std::string_view, std::string_view>, 11> _mime_data {{
        { "text/html",              ".html"       },
        { "text/html",              ".css"        },
        { "text/plain",             ".txt"        },
        { "application/javascript", ".javascript" },
        { "application/json",       ".json"       },
        { "application/pdf",        ".pdf"        },
        { "image/png",              ".png"        },
        { "image/gif",              ".gif"        },
        { "image/svg+xml",          ".svg"        },
        { "image/jpeg",             ".jpg"        },
        { "text/plain",             ""            } // for extensionless files
        // ... more mappings can be added
    }};

    // Maps for fast lookups
    static const std::unordered_map<std::string_view, std::string_view> _mimeToExt;
    static const std::unordered_map<std::string_view, std::string_view> _extToMime;

    // Initializer functions (C++ doesn't support static constructors)
    static std::unordered_map<std::string_view, std::string_view> generateMimeToExt();
    static std::unordered_map<std::string_view, std::string_view> generateExtToMime();
};
