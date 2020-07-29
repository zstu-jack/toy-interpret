#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <fstream>

std::string read_from_file(const char* filename);

void filter_char(std::string& text, const char c);

#endif //UTILS_H

