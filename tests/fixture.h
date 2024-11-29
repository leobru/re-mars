#ifndef MARS_TESTS_UTIL_H
#define MARS_TESTS_UTIL_H

#include <gtest/gtest.h>

#include <string>

//
// Read file contents and return it as a string.
//
std::string file_contents(const std::string &filename);

//
// Read file contents as vector of strings.
//
std::vector<std::string> file_contents_split(const std::string &filename);

//
// Split multi-line text as vector of strings.
//
std::vector<std::string> multiline_split(const std::string &multiline);

//
// Read FILE* stream contents and return it as a string.
//
std::string stream_contents(FILE *input);

//
// Create file with given contents.
//
void create_file(const std::string &filename, const std::string &contents);
void create_file(const std::string &dest_filename, const std::string &prolog,
                 const std::string &src_filename, const std::string &epilog);

//
// Check whether string starts with given prefix.
//
bool starts_with(const std::string &str, const char *prefix);

//
// Compare output of simulation.
// Ignore footer.
//
void check_output(const std::string &output_str, const std::string &expect_str);

//
// Convert a 1-word string to the BESM-6 compatible format
// (big-endian) for ease of comparison.
//
inline std::string tobesm(std::string s) {
    s += "     ";
    s.resize(6);
    std::reverse(s.begin(), s.end());
    s.resize(8);
    return s;
}


#endif // MARS_TESTS_UTIL_H
