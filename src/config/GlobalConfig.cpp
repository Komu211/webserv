#include "GlobalConfig.hpp"

// Main parameterized constructor
GlobalConfig::GlobalConfig(std::string file_name)
{
    // Can optionally add a help flag by checking if file_name == "--help"

    // Open config file
    std::ifstream file_stream{file_name};
    if (!file_stream)
        throw std::runtime_error("Could not open config file for reading: " + std::string{strerror(errno)});

    // Parse file, set Global Config, create ServerConfig objects, which in Turn create LocationConfig objects
    parseConfFile(file_stream);
}

/* Getters */

const std::string &GlobalConfig::getRoot() const
{
    return _root;
}

const std::vector<std::string> &GlobalConfig::getIndexFiles() const
{
    return _index_files_vec;
}

std::size_t GlobalConfig::getClientMaxBodySize() const
{
    return _client_max_body_size;
}

bool GlobalConfig::getAutoIndex() const
{
    return _autoindex;
}

const std::map<int, std::string> &GlobalConfig::getErrorPagesMap() const
{
    return _error_pages_map;
}

const std::vector<ServerConfig> &GlobalConfig::getServerConfigs() const
{
    return _serverConfigs;
}

/* Parsing logic */

void GlobalConfig::parseConfFile(std::ifstream &file_stream)
{
    // Save entire file in a string
    std::string fileContents{iFStreamToString(file_stream)};

    int         curlyLevel{0};
    std::size_t directiveStartPos{0};
    std::size_t directiveEndPos{std::string::npos};

    for (std::size_t index{0}; index < fileContents.length(); ++index)
    {
        if (curlyLevel == 0 && fileContents[index] == ';')
        {
            directiveEndPos = index;
            setConfigurationValue(fileContents.substr(directiveStartPos, index - directiveStartPos + 1));
            directiveStartPos = index + 1;
            continue;
        }
        if (fileContents[index] == '{')
            ++curlyLevel;
        else if (fileContents[index] == '}')
        {
            --curlyLevel;
            if (curlyLevel == 0)
            {
                directiveEndPos = index;
                setConfigurationValue(fileContents.substr(directiveStartPos, index - directiveStartPos + 1));
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
    if (directiveEndPos != fileContents.length() - 1)
    {
        std::string trailingContent{fileContents.substr(directiveEndPos + 1)};
        trim(trailingContent);
        if (!trailingContent.empty())
            throw std::runtime_error("Config file syntax error: Unexpected trailing content: " + trailingContent);
    }
    if (_serverConfigsStr.empty())
        throw std::runtime_error("Config file error: At least one server block must be provided");

    // Set ServerConfigs using the strings saved earlier
    for (auto &elem : _serverConfigsStr)
        _serverConfigs.emplace_back(ServerConfig(elem, *this));

    // * Maybe warn if any host:port - server_name pairs are duplicate
}

void GlobalConfig::setConfigurationValue(std::string directive)
{
    trim(directive);

    std::string server{"server"};
    std::string root{"root"};
    std::string client_max_body_size{"client_max_body_size"};
    std::string autoindex{"autoindex"};
    std::string error_page{"error_page"};
    std::string index{"index"};

    std::size_t nextWordPos;

    // Set server config (just save strings for now)
    if (firstWordEquals(directive, server, &nextWordPos))
        _serverConfigsStr.push_back(directive.substr(nextWordPos));
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
        throw std::runtime_error("Config file syntax error: Disallowed directive in global context: " + directive);
}

/* Setters (used by parser) */

void GlobalConfig::setRoot(std::string directive)
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

void GlobalConfig::setClientMaxBodySize(std::string directive)
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

void GlobalConfig::setAutoIndex(std::string directive)
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

void GlobalConfig::setErrorPage(std::string directive)
{
    trim(directive, ";");

    std::vector<std::string> args{splitStrExceptQuotes(directive)};

    if (args.size() < 2)
        throw std::runtime_error("Config file syntax error: Invalid 'error_page' directive value: " + directive);

    std::string errorPageURI{args.back()};
    args.pop_back();

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

void GlobalConfig::setIndex(std::string directive)
{
    trim(directive, ";");

    std::vector<std::string> args{splitStrExceptQuotes(directive)};

    if (args.empty())
        throw std::runtime_error("Config file syntax error: 'index' directive invalid number of arguments: " + directive);

    // Remove default index
    if (!_seen_index)
        _index_files_vec.clear();
    _seen_index = true;

    for (auto &elem : args)
    {
        _index_files_vec.push_back(elem);
    }
}
