#include <string>
#include <algorithm>

#include "utils.h"
#include "define.h"

std::string read_from_file(const char* filename){
    ASSERT_EXIT(filename != nullptr, "filename == nullptr");
    std::ifstream is (filename, std::ifstream::binary);
    if (is) {
        return std::string((std::istreambuf_iterator<char>(is)),
                        std::istreambuf_iterator<char>());
    }
    ASSERT_EXIT(0, "file not exist");
}

void remove_char(std::string& text, const char c){
    text.erase(std::remove(text.begin(), text.end(), c), text.end());
}
