#include "utils.hpp"

// Helpers

std::string iFStreamToString(std::ifstream &file_stream)
{
    std::string result{""};
    std::string line;
    while (std::getline(file_stream, line))
    {
        // Remove comments
        auto commentStart{line.find('#')};
        if (commentStart != std::string::npos)
            line.erase(commentStart);

        trim(line);

        if (line.empty())
            continue;

        result.append(line);
        result += ' ';
    }
    return result;
}

void trim(std::string &str, const std::string &charset)
{
    const auto firstNonSpace = str.find_first_not_of(charset);

    if (firstNonSpace == std::string::npos)
    {
        str.clear();
        return;
    }

    const auto lastNonSpace = str.find_last_not_of(charset);
    const auto strLen = lastNonSpace - firstNonSpace + 1;

    str = str.substr(firstNonSpace, strLen);
}

std::vector<std::string> splitStr(const std::string &str, const std::string &charset)
{
    std::vector<std::string> result;
    std::size_t              wordStartPos{0};

    for (std::size_t i{0}; i < str.length(); ++i)
    {
        if (i + 1 == str.length())
        {
            result.push_back(str.substr(wordStartPos));
            wordStartPos = i + 1;
        }
        else if (charset.find(str[i]) != std::string::npos)
        {
            result.push_back(str.substr(wordStartPos, i - wordStartPos));
            ++i;
            while (charset.find(str[i]) != std::string::npos)
                ++i;
            wordStartPos = i;
        }
    }
    return result;
}

std::vector<std::string> splitStrExceptQuotes(const std::string &str, const std::string &charset)
{
    std::vector<std::string> result;
    std::string              currentToken;
    char                     in_quote{0};
    bool                     escaped{false};

    for (char c : str)
    {
        if (escaped)
        {
            currentToken += c;
            escaped = false;
        }
        else if (c == '\\')
        {
            escaped = true;
        }
        else if (in_quote)
        {
            if (c == in_quote)
            {
                in_quote = 0;
            }
            else
            {
                currentToken += c;
            }
        }
        else if (c == '\'' || c == '\"')
        {
            in_quote = c;
        }
        else if (charset.find(c) != std::string::npos)
        {
            if (!currentToken.empty())
            {
                result.push_back(currentToken);
                currentToken.clear();
            }
        }
        else
        {
            currentToken += c;
        }
    }

    if (in_quote)
        throw std::runtime_error("Unclosed or unexpected quote in config file");
    if (escaped)
        throw std::runtime_error("Invalid escape character in config file");

    if (!currentToken.empty())
        result.push_back(currentToken);

    return result;
}

void trimOuterSpacesAndQuotes(std::string &str)
{
    trim(str);

    if (str.empty())
        return;

    if (str[0] != '\'' && str[0] != '"')
        return; // No outer quotes to trim

    if (str.length() < 2) // opening quote is last char
        return;

    if (str.back() != str.front()) // Closing quote is not at the end
        return;

    // Finally remove the outer quotes
    str.erase(0, 1);
    str.erase(str.length() - 1, 1);
}

bool firstWordEquals(const std::string &str, const std::string &comparison, std::size_t *next_word_pos)
{
    if (str.compare(0, comparison.length(), comparison) == 0 && std::isspace(str.at(comparison.length())))
    {
        if (next_word_pos)
            *next_word_pos = comparison.length() + 1;
        return true;
    }

    std::string singQuoted{"'" + comparison + "'"};
    if (str.compare(0, singQuoted.length(), singQuoted) == 0 && std::isspace(str.at(singQuoted.length())))
    {
        if (next_word_pos)
            *next_word_pos = singQuoted.length() + 1;
        return true;
    }

    std::string doubQuoted{"\"" + comparison + "\""};
    if (str.compare(0, doubQuoted.length(), doubQuoted) == 0 && std::isspace(str.at(doubQuoted.length())))
    {
        *next_word_pos = doubQuoted.length() + 1;
        return true;
    }

    if (next_word_pos)
        *next_word_pos = 0;
    return false;
}

bool strEndsWith(const std::string& str, const std::string& suffix)
{
    if (str.length() < suffix.length())
        return false;
    return str.substr(str.length() - suffix.length()) == suffix;
}

bool isHttpMethod(const std::string &str)
{
    static const std::set<std::string> http_methods = {"get",     "post",  "delete", "put",    "head",
                                                       "options", "patch", "trace",  "connect"};
    return http_methods.count(str) > 0;
}

bool isStandardAddress(const std::string &address)
{
    if (address == "localhost" || address == "0.0.0.0" || address == "::")
        return true;

    if (address.length() == 9 && address.compare(0, 4, "127.") == 0)
        return true;

    return false;
}

std::string getCurrentGMTString()
{
    // Get current time as time_point
    auto now = std::chrono::system_clock::now();

    // Convert to time_t for use with gmtime
    std::time_t time_t = std::chrono::system_clock::to_time_t(now);

    // Convert to GMT/UTC tm struct
    std::tm *gmt_tm = std::gmtime(&time_t);

    // Format using put_time
    std::ostringstream oss;
    oss << std::put_time(gmt_tm, "%a, %d %b %Y %H:%M:%S GMT");

    return oss.str();
}

std::string getLastModTimeHTTP(const std::filesystem::path &filePath)
{
    try
    {
        if (std::filesystem::exists(filePath))
        {
            // Get last modification time in fileclock format
            auto ftime = std::filesystem::last_write_time(filePath);

            // Convert to system_clock time_point
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());

            // Convert to time_t for use with gmtime
            std::time_t time_t = std::chrono::system_clock::to_time_t(sctp);

            // Convert to GMT/UTC tm struct
            std::tm *gmt_tm = std::gmtime(&time_t);

            // Format using put_time
            std::ostringstream oss;
            oss << std::put_time(gmt_tm, "%a, %d %b %Y %H:%M:%S GMT");

            return oss.str();
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Error getting last modification time for " << filePath.filename() << ": " << e.what() << '\n';
    }
    return "Unknown"; // Generally should not be reached
}

std::string readFileToString(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file)
    {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string bytesToHumanReadable(std::size_t size)
{
    int o{};
    std::ostringstream oss{};
    double mantissa = size;
    for (; mantissa >= 1024.; mantissa /= 1024., ++o);
    oss << std::ceil(mantissa * 10.) / 10. << "BKMGTPE"[o];
    // o ? oss << "B (" << size << ')' : oss;
    return oss.str();
}

std::string reasonPhraseFromStatusCode(int code)
{
    switch (code)
    {
    // 1xx - Informational
    case 100:
        return "Continue";
    case 101:
        return "Switching Protocols";
    case 102:
        return "Processing";
    case 103:
        return "Early Hints";

    // 2xx - Successful
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 202:
        return "Accepted";
    case 203:
        return "Non-Authoritative Information";
    case 204:
        return "No Content";
    case 205:
        return "Reset Content";
    case 206:
        return "Partial Content";
    case 207:
        return "Multi-Status";
    case 208:
        return "Already Reported";
    case 226:
        return "IM Used";

    // 3xx - Redirection
    case 300:
        return "Multiple Choices";
    case 301:
        return "Moved Permanently";
    case 302:
        return "Found";
    case 303:
        return "See Other";
    case 304:
        return "Not Modified";
    case 305:
        return "Use Proxy";
    case 307:
        return "Temporary Redirect";
    case 308:
        return "Permanent Redirect";

    // 4xx - Client Error
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 402:
        return "Payment Required";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 406:
        return "Not Acceptable";
    case 407:
        return "Proxy Authentication Required";
    case 408:
        return "Request Timeout";
    case 409:
        return "Conflict";
    case 410:
        return "Gone";
    case 411:
        return "Length Required";
    case 412:
        return "Precondition Failed";
    case 413:
        return "Content Too Large";
    case 414:
        return "URI Too Long";
    case 415:
        return "Unsupported Media Type";
    case 416:
        return "Range Not Satisfiable";
    case 417:
        return "Expectation Failed";
    case 418:
        return "I'm a teapot";
    case 421:
        return "Misdirected Request";
    case 422:
        return "Unprocessable Content";
    case 423:
        return "Locked";
    case 424:
        return "Failed Dependency";
    case 425:
        return "Too Early";
    case 426:
        return "Upgrade Required";
    case 428:
        return "Precondition Required";
    case 429:
        return "Too Many Requests";
    case 431:
        return "Request Header Fields Too Large";
    case 451:
        return "Unavailable For Legal Reasons";

    // 5xx - Server Error
    case 500:
        return "Internal Server Error";
    case 501:
        return "Not Implemented";
    case 502:
        return "Bad Gateway";
    case 503:
        return "Service Unavailable";
    case 504:
        return "Gateway Timeout";
    case 505:
        return "HTTP Version Not Supported";
    case 506:
        return "Variant Also Negotiates";
    case 507:
        return "Insufficient Storage";
    case 508:
        return "Loop Detected";
    case 510:
        return "Not Extended";
    case 511:
        return "Network Authentication Required";

    default:
        return std::string();
    }
}
