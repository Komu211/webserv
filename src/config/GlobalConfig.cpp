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

void setConfigurationValue(const std::string &currentDirective)
{
    // ! set configuration value
    std::cout << currentDirective << '\n'; // ! test
}

void GlobalConfig::parseConfFile(std::ifstream &file_stream)
{
    // Save entire file in a string
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
        std::string trailingContent {fileContents.substr(directiveEndPos + 1)};
        trimWhitespace(trailingContent);
        if (!trailingContent.empty())
            throw std::runtime_error("Config file syntax error: Unexpected trailing content: " + trailingContent);
    }
}

// Helper
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