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

void GlobalConfig::parseConfFile(std::ifstream &file_stream)
{
    // Save entire file in a string // * Can be made into a helper function
    std::string line;
    std::string fileContents{""};
    while (std::getline(file_stream, line))
    {
        // Remove comments
        auto commentStart{line.find('#')};
        if (commentStart != std::string::npos)
            line.erase(commentStart);

        trimWhitespace(line);

        if (line.empty())
            continue;

        fileContents.append(line);
        fileContents += ' ';
    }

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
        trimWhitespace(trailingContent);
        if (!trailingContent.empty())
            throw std::runtime_error("Config file syntax error: Unexpected trailing content: " + trailingContent);
    }
    if (_serverConfigs.empty())
        throw std::runtime_error("At least one server block must be provided in the config file");
}

void GlobalConfig::setConfigurationValue(std::string directive)
{
    trimWhitespace(directive);

    std::string server{"server"};
    std::string root{"root"};
    std::string client_max_body_size{"client_max_body_size"};
    std::string autoindex{"autoindex"};
    std::string error_page{"error_page"};
    std::string index{"index"};

    // Set server config
    if (directive.compare(0, server.length(), server) == 0)
        _serverConfigs.emplace_back(ServerConfig(directive, *this));
    // Set root
    else if (directive.compare(0, root.length(), root) == 0)
        setRoot(directive.substr(root.length() + 1));
    // Set client_max_body_size
    else if (directive.compare(0, client_max_body_size.length(), client_max_body_size) == 0)
        setClientMaxBodySize(directive.substr(client_max_body_size.length() + 1));
    // Set autoindex on or off
    else if (directive.compare(0, autoindex.length(), autoindex) == 0)
        setAutoIndex(directive.substr(autoindex.length() + 1));
    // ! _error_pages_map todo
    // ! _index_files_vec todo
    else
        throw std::runtime_error("Config file syntax error: Disallowed directive in global context: " + directive);
}

/* Setters (used by parser) */

void GlobalConfig::setRoot(std::string directive)
{
    trimWhitespace(directive, "; \t\n\r\f\v");
    for (std::size_t i{0}; i < directive.size(); ++i)
    {
        if (std::isspace(directive[i]) && i > 0 && directive[i - 1] != '\\')
            throw std::runtime_error("'root' directive must not have more than one argument: " + directive);
    }
    _root = directive;
}

void GlobalConfig::setClientMaxBodySize(std::string directive)
{
    trimWhitespace(directive, "; \t\n\r\f\v");
    for (std::size_t i{0}; i < directive.size(); ++i)
    {
        if (std::isspace(directive[i]) && i > 0 && directive[i - 1] != '\\')
            throw std::runtime_error("'client_max_body_size' directive must not have more than one argument: " + directive);
    }

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
        throw std::runtime_error("Invalid 'client_max_body_size' directive value: " + directive);
    }

    if (remainingPos != directive.length())
        throw std::runtime_error("Invalid 'client_max_body_size' directive value: " + directive);

    // * This can be removed if we can ensure that 0 size means no size checking by the server
    if (_client_max_body_size == 0)
        _client_max_body_size = std::numeric_limits<std::size_t>::max();
}

void GlobalConfig::setAutoIndex(std::string directive)
{
    trimWhitespace(directive, "; \t\n\r\f\v");

    // Convert string to lowercase
    std::transform(directive.begin(), directive.end(), directive.begin(), [](unsigned char c) { return std::tolower(c); });

    if (directive == "on")
        _autoindex = true;
    else if (directive == "off")
        _autoindex = false;
    else
        throw std::runtime_error("Invalid 'autoindex' directive value: " + directive);
}

// Helpers

void trimWhitespace(std::string &str, const std::string &whitespace)
{
    const auto firstNonSpace = str.find_first_not_of(whitespace);

    if (firstNonSpace == std::string::npos)
    {
        str.clear();
        return;
    }

    const auto lastNonSpace = str.find_last_not_of(whitespace);
    const auto strLen = lastNonSpace - firstNonSpace + 1;

    str = str.substr(firstNonSpace, strLen);
}

// std::vector<std::string> splitStr(const std::string &str, const std::string &whitespace = " \t\n\r\f\v")
// {
//     std::size_t wordStartPos;
//     // ! Todo
// }