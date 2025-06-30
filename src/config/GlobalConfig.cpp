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
    int         lineNum{0};
    bool        directiveEnd{false};
    int         curlyLevel{0};
    std::string currentDirective{""};

    for (std::string line; std::getline(file_stream, line);)
    {
        ++lineNum;

        // Remove comments
        auto commentStart {line.find('#')};
        if (commentStart != std::string::npos)
            line.erase(commentStart);

        trimWhitespace(line);

        if (line.empty())
            continue;

        std::size_t directiveEndPos {std::string::npos};
        for (auto it{line.begin()}; it != line.end(); ++it)
        {
            if ( curlyLevel == 0 && *it == ';')
            {
                directiveEndPos = it - line.begin();
                directiveEnd = true;
                currentDirective.append(line, 0, directiveEndPos + 1);
                break;
            }
            if (*it == '{')
                ++curlyLevel;
            else if (*it == '}')
            {
                --curlyLevel;
                if (curlyLevel == 0)
                {
                    directiveEndPos = it - line.begin();
                    directiveEnd = true;
                    currentDirective.append(line, 0, directiveEndPos + 1);
                    break;
                }
                else if (curlyLevel < 0)
                    throw std::runtime_error("Config file syntax error: Unexpected '}' on line " + std::to_string(lineNum));
            }
            //
        }

        if (directiveEnd)
        {
            // ! set configuration value
            std::cout << "---Global directive found:---" << '\n'; // ! test
            std::cout << currentDirective << '\n'; // ! test
            std::cout << "----end of directive----" << '\n'; // ! test
            
            if (directiveEndPos + 1 < line.length())
                currentDirective = line.substr(directiveEndPos + 1);
            else
                currentDirective = "";

            directiveEnd = false;
        }
        else
            currentDirective.append(line);
    }
    if (curlyLevel > 0)
        throw std::runtime_error("Config file syntax error: Unclosed '{'");
    else if (curlyLevel < 0)
        throw std::runtime_error("Config file syntax error: Unexpected '}'");
    //
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