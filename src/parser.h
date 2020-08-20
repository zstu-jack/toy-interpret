#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <algorithm>
#include "define.h"

class Token{
public:
    TokenType token_type_;
    std::string values_;
    int lines_;
    Token(TokenType token_type_, const std::string& values_);
    Token(TokenType token_type_, const std::string& values_, int line);
};

struct Tokenizer{
    Tokenizer();

    void incp(size_t& p, size_t offset = 1);
    void parse(const std::string& text);
    void print();
    void print_lines();

    bool is_symbol_start(char c);
    bool is_digit(char c);
    bool is_symbol(char c);
    bool is_operator(char c);
    bool pass_linefeed();

    std::vector<Token*> tokens_;

    std::string program_;
    std::map<int, std::string> line2line_str_;

    size_t _p, _lp, _line, _linep_start;
};



#endif //LUA_PARSER_H
