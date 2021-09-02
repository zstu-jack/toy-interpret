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

    void move(size_t offset = 1);
    bool linefeed();
    char now();
    char nxt();
    void parse(const std::string& text);

    void print_tokens();
    void print_line_tokens();

    bool is_symbol_start(char c);
    bool is_digit(char c);
    bool is_symbol(char c);
    bool is_operator(char c);

    std::vector<Token> _tokens;

    std::string _program;
    std::map<int, std::string> _line2line_str;

    size_t _pos;
    size_t _line_number, _line_start_pos;
};



#endif //LUA_PARSER_H
