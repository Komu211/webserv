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

// ! Need to update default behavior to not open single or double quotes
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

void trimOuterSpacesAndQuotes(std::string& str)
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
