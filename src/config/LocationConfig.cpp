#include "LocationConfig.hpp"

// Main parameterized ctor (parses location_block_str, inherits the rest from server_config)
LocationConfig::LocationConfig(const std::string &location_block_str, const ServerConfig &server_config)
    : _root{server_config.getRoot()}
    , _index_files_vec{server_config.getIndexFilesVec()}
    , _client_max_body_size{server_config.getClientMaxBodySize()}
    , _autoindex{server_config.getAutoIndex()}
    , _error_pages_map{server_config.getErrorPagesMap()}
    , _cgi_handlers_map{server_config.getCGIHandlersMap()}
{
    parseLocationConfig(location_block_str);
}

/* Getters */

const std::string &LocationConfig::getRoot() const
{
    return _root;
}

const std::vector<std::string> &LocationConfig::getIndexFilesVec() const
{
    return _index_files_vec;
}

std::size_t LocationConfig::getClientMaxBodySize() const
{
    return _client_max_body_size;
}

bool LocationConfig::getAutoIndex() const
{
    return _autoindex;
}

const std::map<int, std::string> &LocationConfig::getErrorPagesMap() const
{
    return _error_pages_map;
}

const std::set<std::string> &LocationConfig::getLimitExcept() const
{
    return _limit_except;
}

const std::string &LocationConfig::getUploadStore() const
{
    return _upload_store;
}

const std::pair<int, std::string> &LocationConfig::getReturn() const
{
    return _return;
}

const std::map<std::string, std::string> &LocationConfig::getCGIHandlersMap() const
{
    return _cgi_handlers_map;
}

/* Parsing logic */

void LocationConfig::parseLocationConfig(std::string location_block_str)
{
    trim(location_block_str, " \t\n\r\f\v");
    if (location_block_str.front() != '{' || location_block_str.back() != '}' || location_block_str.length() < 2)
        throw std::runtime_error("Config file syntax error: 'location' directive's second argument should be enclosed "
                                 "in "
                                 "a single {}: " +
                                 location_block_str);

    // Remove the braces
    location_block_str.erase(0, 1);
    location_block_str.erase(location_block_str.length() - 1, 1);

    trim(location_block_str, " \t\n\r\f\v");

    int         curlyLevel{0};
    std::size_t directiveStartPos{0};
    std::size_t directiveEndPos{std::string::npos};

    for (std::size_t index{0}; index < location_block_str.length(); ++index)
    {
        if (curlyLevel == 0 && location_block_str[index] == ';')
        {
            directiveEndPos = index;
            setConfigurationValue(location_block_str.substr(directiveStartPos, index - directiveStartPos + 1));
            directiveStartPos = index + 1;
            continue;
        }
        if (location_block_str[index] == '{')
            ++curlyLevel;
        else if (location_block_str[index] == '}')
        {
            --curlyLevel;
            if (curlyLevel == 0)
            {
                directiveEndPos = index;
                setConfigurationValue(location_block_str.substr(directiveStartPos, index - directiveStartPos + 1));
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
    if (directiveEndPos != location_block_str.length() - 1)
    {
        std::string trailingContent{location_block_str.substr(directiveEndPos + 1)};
        trim(trailingContent);
        if (!trailingContent.empty())
            throw std::runtime_error("Config file syntax error: Unexpected trailing content: " + trailingContent);
    }
}

void LocationConfig::setConfigurationValue(std::string directive)
{
    trim(directive);

    std::string root{"root"};
    std::string client_max_body_size{"client_max_body_size"};
    std::string autoindex{"autoindex"};
    std::string error_page{"error_page"};
    std::string cgi_handler{"cgi_handler"};
    std::string index{"index"};
    std::string limit_except{"limit_except"};
    std::string upload_store{"upload_store"};
    std::string return_directive{"return"};

    std::size_t nextWordPos;

    // TODO: CGI handler
    // Set root
    if (firstWordEquals(directive, root, &nextWordPos))
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
    // Set CGI handler
    else if (firstWordEquals(directive, cgi_handler, &nextWordPos))
        setCGIHandler(directive.substr(nextWordPos));
    // Set index files
    else if (firstWordEquals(directive, index, &nextWordPos))
        setIndex(directive.substr(nextWordPos));
    // Set limit except
    else if (firstWordEquals(directive, limit_except, &nextWordPos))
        setLimitExcept(directive.substr(nextWordPos));
    // Set upload storage location
    else if (firstWordEquals(directive, upload_store, &nextWordPos))
        setUploadStore(directive.substr(nextWordPos));
    // Set return directive
    else if (firstWordEquals(directive, return_directive, &nextWordPos))
        setReturn(directive.substr(nextWordPos));
    else
        throw std::runtime_error("Config file syntax error: Disallowed directive in location context: " + directive);
}

/* Setters (used by parser) */

void LocationConfig::setRoot(std::string directive)
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

void LocationConfig::setClientMaxBodySize(std::string directive)
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

void LocationConfig::setAutoIndex(std::string directive)
{
    if (_seen_autoindex)
        throw std::runtime_error("Config file syntax error: 'autoindex' directive is duplicate: " + directive);

    trim(directive, ";");
    trimOuterSpacesAndQuotes(directive);
    ;

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

void LocationConfig::setErrorPage(std::string directive)
{
    trim(directive, ";");

    std::vector<std::string> args{splitStrExceptQuotes(directive)};

    if (args.size() < 2)
        throw std::runtime_error("Config file syntax error: Invalid 'error_page' directive value: " + directive);

    std::string errorPageURI{args.back()};
    args.pop_back();

    // (it would be wrong to) Remove any error pages inherited from server context to override them
    // if (!_seen_error_page)
    //     _error_pages_map.clear();
    // _seen_error_page = true;

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

void LocationConfig::setCGIHandler(std::string directive)
{
    trim(directive, ";");

    std::vector<std::string> args{splitStrExceptQuotes(directive)};

    if (args.size() != 2)
        throw std::runtime_error("Config file syntax error: 'cgi_handler' directive invalid number of arguments: " + directive);

    std::string extension{args[0]};
    std::string interpreter{args[1]};

    if (extension.empty() || interpreter.empty())
        throw std::runtime_error("Config file syntax error: Invalid 'cgi_handler' directive argument(s): " + directive);

    if (extension[0] != '.')
        extension.insert(0, 1, '.');

    if (_seen_cgi_handler && _cgi_handlers_map.find(extension) != _cgi_handlers_map.end())
        throw std::runtime_error("Config file syntax error: 'cgi_handler' directive is duplicate: " + directive);

    if (access(interpreter.c_str(), X_OK) != 0)
        throw std::runtime_error("Config file error: 'cgi_handler' interpreter either does not exist or is not "
                                 "executable: " +
                                 directive);

    _cgi_handlers_map[extension] = interpreter;
    _seen_cgi_handler = true;
}

void LocationConfig::setIndex(std::string directive)
{
    trim(directive, ";");

    std::vector<std::string> args{splitStrExceptQuotes(directive)};

    if (args.empty())
        throw std::runtime_error("Config file syntax error: 'index' directive invalid number of arguments: " + directive);

    // Remove any index files inherited from server context to override them
    if (!_seen_index)
        _index_files_vec.clear();
    _seen_index = true;

    for (auto &elem : args)
    {
        _index_files_vec.push_back(elem);
    }
}

void LocationConfig::setLimitExcept(std::string directive)
{
    if (_seen_limit_except)
        throw std::runtime_error("Config file syntax error: 'limit_except' directive is duplicate: " + directive);

    trim(directive, ";");

    std::vector<std::string> args{splitStrExceptQuotes(directive)};

    if (args.empty())
        throw std::runtime_error("Config file syntax error: 'limit_except' directive invalid number of arguments: " + directive);

    for (auto &elem : args)
    {
        // convert to lower case
        std::transform(elem.begin(), elem.end(), elem.begin(), [](unsigned char c) { return std::tolower(c); });

        // check if HTTP method is valid
        if (!isHttpMethod(elem)) // * Currently possible to allow methods other than GET, POST, and DELETE
            throw std::runtime_error("Config file syntax error: 'limit_except' directive invalid method: " + elem);

        _limit_except.insert(elem);
    }
}

void LocationConfig::setUploadStore(std::string directive)
{
    if (_seen_upload_store)
        throw std::runtime_error("Config file syntax error: 'upload_store' directive is duplicate: " + directive);

    trim(directive, ";");

    std::vector<std::string> args{splitStrExceptQuotes(directive)};

    if (args.size() != 1)
        throw std::runtime_error("Config file syntax error: 'upload_store' directive invalid number of arguments: " + directive);

    _upload_store = args[0];
    _seen_upload_store = true;
}

void LocationConfig::setReturn(std::string directive)
{
    if (_seen_return)
        throw std::runtime_error("Config file syntax error: 'return' directive is duplicate: " + directive);

    trim(directive, ";");

    std::vector<std::string> args{splitStrExceptQuotes(directive)};

    if (args.size() > 2)
        throw std::runtime_error("Config file syntax error: Invalid number of arguments in 'return' directive: " + directive);

    std::size_t remainingPos;
    try
    {
        _return.first = std::stoi(args[0], &remainingPos);
    }
    catch (const std::exception &)
    {
        throw std::runtime_error("Config file syntax error: 'return' directive first argument should be numeric "
                                 "(code): " +
                                 directive);
    }
    if (remainingPos < args[0].length()) // Not all characters in the string are numeric
        throw std::runtime_error("Config file syntax error: 'return' directive first argument should be numeric "
                                 "(code): " +
                                 directive);
    if (_return.first < 0 || _return.first > 999)
        throw std::runtime_error("Config file syntax error: 'return' directive invalid return code: " + directive);

    if (args.size() == 2)
        _return.second = args[1];

    _seen_return = true;
}
