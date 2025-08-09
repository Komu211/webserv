#include <string>
#include <unordered_map>
#include <utility>
#include "utils.hpp"

/* Converts a given status code, headers map and response body into a properly formatted HTTP/1.1 response
Automatically adds the "Date", "Server", and "Content-Length" headers, but they can be provided to override default values */
class ResponseWriter
{
public:
    ResponseWriter() = delete;
    ResponseWriter(const ResponseWriter &src) = delete;
    ResponseWriter(ResponseWriter &&src) = default;
    ResponseWriter &operator=(const ResponseWriter &src) = delete;
    ResponseWriter &operator=(ResponseWriter &&src) = delete;
    ~ResponseWriter() = default;

    explicit ResponseWriter(int statusCode, const std::unordered_map<std::string, std::string>& headers, const std::string& response_body);

    std::string write() const;

private:
    std::string _start_line;
    std::unordered_map<std::string, std::string> _headers;
    const std::string& _response_body;
};
