#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>

/* Trim the start and end of a string using a given charset (default whitespaces) */
void trim(std::string &str, const std::string &charset = " \t\n\r\f\v");

// ! Need to update default behavior to not open single or double quotes
/* Split a string into a vector of "words" using a given delimiter (default whitespaces) */
std::vector<std::string> splitStr(const std::string &str, const std::string &charset = " \t\n\r\f\v");

/* Convert a given input file stream to std::string */
std::string iFStreamToString(std::ifstream &file_stream);

/* Trim the outer spaces and then remove the outermost quotes */
void trimOuterSpacesAndQuotes(std::string& str);

/* Checked if the first word of `str` is equal to `comparison` without regard to the first word being quoted */
bool firstWordEquals(const std::string& str, const std::string& comparison, std::size_t* next_word_pos = nullptr);