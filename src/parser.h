#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <algorithm>

class Token{
public:
    int token_type_;
    std::string values_;
    Token(int token_type_, const std::string& values_);
};

struct Tokenizer{
    void parse(const std::string& text);
    void print();

    bool is_symbol_start(char c);
    bool is_digit(char c);
    bool is_symbol(char c);
    bool is_operator(char c);

    std::vector<Token*> tokens_;
    int consumed_;
};



#endif //LUA_PARSER_H
