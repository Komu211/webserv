#include "ServerConfig.hpp"

// ! To be deleted
// ServerConfig::ServerConfig()
// {
//     _host = "localhost";
//     _port = 8080;
//     _root = ".";
//     _serverNames = {"localhost"};
// }

// Main parameterized ctor (parses server_block_str, inherits the rest from global_config)
ServerConfig::ServerConfig(const std::string &server_block_str, const GlobalConfig &global_config)
    : _root{global_config.getRoot()}
    , _index_files_vec{global_config.getIndexFiles()}
    , _client_max_body_size{global_config.getClientMaxBodySize()}
    , _autoindex{global_config.getAutoIndex()}
    , _error_pages_map{global_config.getErrorPagesMap()}
{
    parseServerConfig(server_block_str);

    // Convert all provided `listen` host:port combinations to `struct addrinfo`; throw on error
    setAddrInfo();

    // Set locationConfigs using the strings saved earlier
    initLocationConfig();
}

ServerConfig::~ServerConfig()
{
    for (auto &elem : _addrinfo_lists_vec)
        freeaddrinfo(elem);
}

/* Getters */

const std::vector<StringPair> &ServerConfig::getHostPortPairs() const
{
    return _listen_host_port;
}

const std::vector<AddrInfoPair> &ServerConfig::getAddrInfoVec() const
{
    return _addrinfo_vec;
}

const std::string &ServerConfig::getRoot() const
{
    return _root;
}

const std::vector<std::string> &ServerConfig::getServerNames() const
{
    return _serverNames;
}

const std::vector<std::string> &ServerConfig::getIndexFilesVec() const
{
    return _index_files_vec;
}

std::size_t ServerConfig::getClientMaxBodySize() const
{
    return _client_max_body_size;
}

bool ServerConfig::getAutoIndex() const
{
    return _autoindex;
}

const std::map<int, std::string> &ServerConfig::getErrorPagesMap() const
{
    return _error_pages_map;
}

const std::map<std::string, std::unique_ptr<LocationConfig>> &ServerConfig::getLocationsMap() const
{
    return _locations_map;
}

// // ! Need to update because server can listen to multiple host:ports combinations
// int ServerConfig::getPort() const
// {
//     return _port;
// }

// // ! remove (not needed)
// void ServerConfig::setPort(int newPort)
// {
//     _port = newPort;
// }

// // ! Need to update because server can listen to multiple host:ports combinations
// std::string ServerConfig::getHost() const
// {
//     return _host;
// }

/* Parsing logic */

void ServerConfig::parseServerConfig(std::string server_block_str)
{
    trim(server_block_str, " \t\n\r\f\v");
    if (server_block_str.front() != '{' || server_block_str.back() != '}' || server_block_str.length() < 2)
        throw std::runtime_error("Config file syntax error: 'server' directive arguments should be enclosed in a "
                                 "single {}: " +
                                 server_block_str);
    // Remove the braces
    server_block_str.erase(0, 1);
    server_block_str.erase(server_block_str.length() - 1, 1);

    trim(server_block_str, " \t\n\r\f\v");

    int         curlyLevel{0};
    std::size_t directiveStartPos{0};
    std::size_t directiveEndPos{std::string::npos};

    for (std::size_t index{0}; index < server_block_str.length(); ++index)
    {
        if (curlyLevel == 0 && server_block_str[index] == ';')
        {
            directiveEndPos = index;
            setConfigurationValue(server_block_str.substr(directiveStartPos, index - directiveStartPos + 1));
            directiveStartPos = index + 1;
            continue;
        }
        if (server_block_str[index] == '{')
            ++curlyLevel;
        else if (server_block_str[index] == '}')
        {
            --curlyLevel;
            if (curlyLevel == 0)
            {
                directiveEndPos = index;
                setConfigurationValue(server_block_str.substr(directiveStartPos, index - directiveStartPos + 1));
                directiveStartPos = index + 1;
                continue;
            }
            else if (curlyLevel < 0)
                throw std::runtime_error("Config file syntax error: Unexpected '}'");
        }
    }
    if (curlyLevel > 0)
        throw std::runtime_error("Config file syntax error: Unclosed '{'");
    else if (curlyLevel < 0)
        throw std::runtime_error("Config file syntax error: Unexpected '}'");
    if (directiveEndPos != server_block_str.length() - 1)
    {
        std::string trailingContent{server_block_str.substr(directiveEndPos + 1)};
        trim(trailingContent);
        if (!trailingContent.empty())
            throw std::runtime_error("Config file syntax error: Unexpected trailing content: " + trailingContent);
    }
}

void ServerConfig::setConfigurationValue(std::string directive)
{
    trim(directive);

    std::string listen{"listen"};
    std::string server_name{"server_name"};
    std::string location{"location"};
    // TODO std::string cgi_handler{"cgi_handler"};
    std::string root{"root"};
    std::string client_max_body_size{"client_max_body_size"};
    std::string autoindex{"autoindex"};
    std::string error_page{"error_page"};
    std::string index{"index"};

    std::size_t nextWordPos;

    // TODO: CGI handler
    // Set listening host:port(s) for this server
    if (firstWordEquals(directive, listen, &nextWordPos))
        setListen(directive.substr(nextWordPos));
    // Set server names
    else if (firstWordEquals(directive, server_name, &nextWordPos))
        setServerName(directive.substr(nextWordPos));
    // Set location config (just save strings for now)
    else if (firstWordEquals(directive, location, &nextWordPos))
        _locationConfigsStr.push_back(directive.substr(nextWordPos));
    // Set root
    else if (firstWordEquals(directive, root, &nextWordPos))
        setRoot(directive.substr(nextWordPos));
    // Set client_max_body_size
    else if (firstWordEquals(directive, client_max_body_size, &nextWordPos))
        setClientMaxBodySize(directive.substr(nextWordPos));
    // Set autoindex on or off
    else if (firstWordEquals(directive, autoindex, &nextWordPos))
        setAutoIndex(directive.substr(nextWordPos));
    // Set error pages
    else if (firstWordEquals(directive, error_page, &nextWordPos))
        setErrorPage(directive.substr(nextWordPos));
    // Set index files
    else if (firstWordEquals(directive, index, &nextWordPos))
        setIndex(directive.substr(nextWordPos));
    else
        throw std::runtime_error("Config file syntax error: Disallowed directive in server context: " + directive);
}

void ServerConfig::setAddrInfo()
{
    if (_listen_host_port.empty()) // This should be impossible but still adding a check in case something goes wrong
        throw std::runtime_error("Fatal: No listening `host:port` available for a server");

    for (const auto &hostPort : _listen_host_port)
    {
        addrinfo *res;
        addrinfo  hints = {};

        hints.ai_flags = AI_PASSIVE;     // Fill in `sockaddr` suitable for bind()
        hints.ai_family = AF_UNSPEC;     // IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; // TCP

        // The below check can take a while for non-standard addresses, so notify the user
        if (!isStandardAddress(hostPort.first))
            std::cout << "Checking validity of " << hostPort.first << ':' << hostPort.second << "...\n";

        // Pass everything to getaddrinfo so it can fill `res` with the corresponding `sockaddr`
        int getAddrReturn{getaddrinfo(hostPort.first.c_str(), hostPort.second.c_str(), &hints, &res)};
        if (getAddrReturn != 0) // getaddrinfo return non-zero means invalid host:port
            throw std::runtime_error("Could not validate `" + hostPort.first + ":" + hostPort.second + "`: " + gai_strerror(getAddrReturn));

        // Save returned list so it can be freed by the destructor
        _addrinfo_lists_vec.push_back(res);

        // Save each `struct addrinfo` to be used with socket(), bind(), listen(), etc. later
        for (auto cur{res}; cur != NULL; cur = cur->ai_next)
            _addrinfo_vec.emplace_back(*cur, hostPort);
    }
}

void ServerConfig::initLocationConfig()
{
    for (auto &elem : _locationConfigsStr)
    {
        auto openingBracePos{elem.find('{')};

        if (openingBracePos == std::string::npos)
            throw std::runtime_error("Config file syntax error: 'location' directive has no opening '{': " + elem);
        else if (openingBracePos < 1)
            throw std::runtime_error("Config file syntax error: 'location' directive invalid number of arguments: " + elem);

        std::string locName{elem.substr(0, openingBracePos)};

        std::vector<std::string> args{splitStrExceptQuotes(locName)};

        if (args.size() != 1)
            throw std::runtime_error("Config file syntax error: 'location' directive invalid number of arguments: " + elem);

        locName = args[0];

        if (_locations_map.find(locName) != _locations_map.end())
            throw std::runtime_error("Config file syntax error: duplicate location: " + locName);

        // _locations_map[locName] = LocationConfig(elem.substr(openingBracePos), *this);
        _locations_map.emplace(locName, std::make_unique<LocationConfig>(elem.substr(openingBracePos), *this));
    }
}

/* Setters (used by parser) */

void ServerConfig::setListen(std::string directive)
{
    trim(directive, ";");

    std::vector<std::string> args{splitStrExceptQuotes(directive)};

    if (args.size() != 1)
        throw std::runtime_error("Config file syntax error: 'listen' directive invalid number of arguments: " + directive);

    directive = args[0];

    // Remove default listen ("0.0.0.0": "80")
    if (!_seen_listen)
        _listen_host_port.clear();
    _seen_listen = true;

    std::string address;
    std::string port;

    if (directive.front() == '[')
    {
        auto endingBracketPos{directive.find_last_of(']')};
        if (endingBracketPos == std::string::npos)
            throw std::runtime_error("Config file syntax error: 'listen' directive invalid value: " + directive);

        address = directive.substr(0, endingBracketPos + 1); // + 1 to convert index to count

        // remove the [] (first and last characters)
        address.erase(0, 1);
        address.erase(address.length() - 1, 1);

        // get the rest of the string
        std::string portStr{};
        if (endingBracketPos < directive.length() - 1)
            portStr = directive.substr(endingBracketPos + 1);

        if (portStr.empty()) // no port has been provided
            port = "80";
        else if (portStr.front() == ':' && portStr.length() > 1)
            port = portStr.substr(1); // Skip the colon and the rest is port
        else
            throw std::runtime_error("Config file syntax error: 'listen' directive invalid value: " + directive);
    }
    else // either ipv4, ipv4:port, or just port
    {
        auto lastColonPos{directive.find_last_of(':')};
        if (lastColonPos != std::string::npos)
        {
            // ipv4:port is provided
            if (lastColonPos == 0 || lastColonPos == directive.length() - 1) // `:` must not be first or last
                throw std::runtime_error("Config file syntax error: 'listen' directive invalid value: " + directive);
            address = directive.substr(0, lastColonPos);
            // * Validity checking can be skipped since it will be done by getaddrinfo
            port = directive.substr(lastColonPos + 1);
        }
        else
        {
            // either only ipv4 or port is provided
            // if it's numeric, it is port
            std::size_t remainingPos;
            int         converted;
            try
            {
                converted = std::stoi(directive, &remainingPos);
            }
            catch (const std::exception &)
            {
                // Assume its only host and no port
                port = "80";
                address = directive;
            }
            if (port.empty()) // Catch block didn't execute
            {
                if (remainingPos < directive.length())
                {
                    // Some non-numeric characters after numeric (assume its only host and no port)
                    port = "80";
                    address = directive;
                }
                else
                {
                    // Only numeric
                    if (converted < 1 || converted > 65535)
                        throw std::runtime_error("Config file syntax error: 'listen' directive invalid port: " + directive);
                    port = std::to_string(converted);
                    address = "0.0.0.0";
                }
            }
        }
    }
    // Ensure that address or port are not empty (shouldn't happen unless listen is [])
    if (address.empty() || port.empty())
        throw std::runtime_error("An error occurred while parsing 'listen' directive: " + directive);

    // Save the address and port in our vector of pairs
    StringPair host_port{address, port};

    // Ensure the host:port combination is not already present
    for (auto &elem : _listen_host_port)
    {
        if (elem == host_port)
            throw std::runtime_error("Config file syntax error: 'listen' directive has duplicate value: " + directive);
    }

    _listen_host_port.emplace_back(std::move(host_port));
}

void ServerConfig::setServerName(std::string directive)
{
    trim(directive, ";");

    std::vector<std::string> args{splitStrExceptQuotes(directive)};

    if (args.empty())
        throw std::runtime_error("Config file syntax error: 'server_name' directive invalid number of arguments: " + directive);

    // * Maybe check and warn if server name is already present (inefficient for vector)

    for (auto &elem : args)
    {
        _serverNames.push_back(elem);
    }
}

void ServerConfig::setRoot(std::string directive)
{
    if (_seen_root)
        throw std::runtime_error("Config file syntax error: 'root' directive is duplicate: " + directive);

    trim(directive, ";");

    std::vector<std::string> args{splitStrExceptQuotes(directive)};

    if (args.size() != 1)
        throw std::runtime_error("Config file syntax error: 'root' directive invalid number of arguments: " + directive);

    _root = args[0];
    _seen_root = true;
}

void ServerConfig::setClientMaxBodySize(std::string directive)
{
    if (_seen_client_max_body_size)
        throw std::runtime_error("Config file syntax error: 'client_max_body_size' directive is duplicate: " + directive);

    trim(directive, ";");

    std::vector<std::string> args{splitStrExceptQuotes(directive)};

    if (args.size() != 1)
        throw std::runtime_error("Config file syntax error: 'client_max_body_size' directive invalid number of "
                                 "arguments: " +
                                 directive);

    directive = args[0];

    auto lastIndex{directive.length() - 1};
    if (directive[lastIndex] == 'k' || directive[lastIndex] == 'K')
    {
        directive.erase(lastIndex);
        directive.append("000");
    }
    else if (directive[lastIndex] == 'm' || directive[lastIndex] == 'M')
    {
        directive.erase(lastIndex);
        directive.append("000000");
    }
    else if (directive[lastIndex] == 'g' || directive[lastIndex] == 'G')
    {
        directive.erase(lastIndex);
        directive.append("000000000");
    }

    std::size_t remainingPos;
    try
    {
        _client_max_body_size = std::stoul(directive, &remainingPos);
    }
    catch (const std::exception &)
    {
        throw std::runtime_error("Config file syntax error: Invalid 'client_max_body_size' directive value: " + directive);
    }

    if (remainingPos != directive.length())
        throw std::runtime_error("Config file syntax error: Invalid 'client_max_body_size' directive value: " + directive);

    // * This can be removed if we can ensure that 0 size means no size checking by the server
    if (_client_max_body_size == 0)
        _client_max_body_size = std::numeric_limits<std::size_t>::max();

    _seen_client_max_body_size = true;
}

void ServerConfig::setAutoIndex(std::string directive)
{
    if (_seen_autoindex)
        throw std::runtime_error("Config file syntax error: 'autoindex' directive is duplicate: " + directive);

    trim(directive, ";");
    trimOuterSpacesAndQuotes(directive);

    // Convert string to lowercase
    std::transform(directive.begin(), directive.end(), directive.begin(), [](unsigned char c) { return std::tolower(c); });

    if (directive == "on")
        _autoindex = true;
    else if (directive == "off")
        _autoindex = false;
    else
        throw std::runtime_error("Config file syntax error: Invalid 'autoindex' directive value: " + directive);
    _seen_autoindex = true;
}

void ServerConfig::setErrorPage(std::string directive)
{
    trim(directive, ";");

    std::vector<std::string> args{splitStrExceptQuotes(directive)};

    if (args.size() < 2)
        throw std::runtime_error("Config file syntax error: Invalid 'error_page' directive value: " + directive);

    std::string errorPageURI{args.back()};
    args.pop_back();

    // Remove any error pages inherited from global context to override them
    if (!_seen_error_page)
        _error_pages_map.clear();
    _seen_error_page = true;

    for (auto &elem : args)
    {
        int         errorNum;
        std::size_t remainingPos;
        try
        {
            errorNum = std::stoi(elem, &remainingPos);
        }
        catch (const std::exception &)
        {
            throw std::runtime_error("Config file syntax error: Invalid 'error_page' directive value: " + directive + ": Not a valid error code: " + elem);
        }
        if (remainingPos != elem.length())
            throw std::runtime_error("Config file syntax error: Invalid 'error_page' directive value: " + directive + ": Not a valid number (error code): " + elem);
        if (errorNum < 300 || errorNum > 599)
            throw std::runtime_error("Config file syntax error: Invalid 'error_page' directive value: " + directive + ": Value must be between 300 and 599: " + elem);
        _error_pages_map[errorNum] = errorPageURI;
    }
}

void ServerConfig::setIndex(std::string directive)
{
    trim(directive, ";");

    std::vector<std::string> args{splitStrExceptQuotes(directive)};

    if (args.empty())
        throw std::runtime_error("Config file syntax error: 'index' directive invalid number of arguments: " + directive);

    // Remove any index files inherited from global context to override them
    if (!_seen_index)
        _index_files_vec.clear();
    _seen_index = true;

    for (auto &elem : args)
    {
        _index_files_vec.push_back(elem);
    }
}
