#include <string>
#include <algorithm>


#include "utils.h"
#include "define.h"

std::string read_from_file(const char* filename){
    ASSERT_EXIT(filename != nullptr, "filename == nullptr");
    std::string text = "";
    std::ifstream is (filename, std::ifstream::binary);
    if (is) {
        is.seekg (0, is.end);
        int32_t length = is.tellg();
        is.seekg (0, is.beg);

        char* buffer = new char [length];
        is.read (buffer,length);
        is.close();

        text = std::string(buffer, length);
        delete[] buffer;
        return text;
    }

    ASSERT_EXIT(0, "file not exist");
}

void filter_char(std::string& text, const char c){
    text.erase(std::remove(text.begin(), text.end(), c), text.end());
}
