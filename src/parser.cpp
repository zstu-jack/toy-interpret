#include <iostream>

#include "parser.h"
#include "define.h"
#include "evaluate.h"

Token::Token(TokenType token_type_, const std::string& values_){
    this->token_type_ = token_type_;
    this->values_ = values_;
}
Token::Token(TokenType token_type_, const std::string& values_, int line){
    this->token_type_ = token_type_;
    this->values_ = values_;
    this->lines_ = line;
}

Tokenizer::Tokenizer(){
    _pos = 0;
    _line_number = 1;
    _line_start_pos = 0;
}


bool Tokenizer::is_symbol_start(char c){ return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
bool Tokenizer::is_digit(char c){ return c >= '0' && c <= '9'; }
bool Tokenizer::is_symbol(char c){ return is_symbol_start(c) || is_digit(c); }
bool Tokenizer::is_operator(char c){ return operators.find(c) != std::string::npos; }
char Tokenizer::now(){ return _program[_pos]; }
char Tokenizer::nxt(){ return _program[_pos + 1]; }
void Tokenizer::move(size_t offset){
    _pos += offset;
    ASSERT_EXIT(_pos <= _program.size(), "exceed program size");
}
bool Tokenizer::linefeed(){
    int move_size = 0;
    if(now() == '\n'){
        move_size = 1;
    }
    if(now() == '\r' && nxt() == '\n'){
        move_size = 2;
    }

    if(move_size == 0){
        return false;
    }
    move(move_size);
    _line2line_str[_line_number ++] = _program.substr(_line_start_pos, _pos - _line_start_pos);
    _line_start_pos = _pos;
    return true;
}

void Tokenizer::parse(const std::string& text){
    ASSERT_EXIT(text.size() > 0, "empty content");
    _program = text;
    for(; _pos != text.size(); ){
        const char& cp = text[_pos];
        size_t start_pos = _pos;

        // symbol or keyword
        if(is_symbol_start(cp)){
            while(_pos < text.size() && is_symbol(text[_pos])) move();

            std::string symbol = text.substr(start_pos, _pos - start_pos);
            if(keywords_2_token_type.count(symbol)) _tokens.push_back(Token(keywords_2_token_type[symbol], symbol, _line_number));
            else _tokens.push_back(Token(TokenType ::SYMBOL, symbol, _line_number));
        }
        // decimal or integer
        else if(is_digit(cp)){
            while(_pos < text.size() && is_digit(text[_pos])) move();
            if(text[_pos] == '.') move();
            while(_pos < text.size() && is_digit(text[_pos])) move();

            std::string number = text.substr(start_pos, _pos - start_pos);
            if(number.find('.') != std::string::npos) _tokens.push_back(Token(TokenType ::DECIMAL, number, _line_number));
            else _tokens.push_back(Token(TokenType ::INTEGER, number, _line_number));
        }
        // operators
        // comments
        else if(is_operator(cp)){
            // operator: char start with `| & ! = > <` may have next char that represent op together.
            // two-char operator:
            //   | -> ||
            //   & -> &&
            //   ! -> !=
            //   = -> ==
            //   > -> >= >>
            //   < -> <= <<
            // comment
            //   //
            //   /*  */
            // single char operators
            //   +
            //   -
            //   /
            //   *
            if(operators_multichar.find(text[_pos]) != std::string::npos) {
                move();
                switch (text[_pos]){
                    case '|':
                        if(text[_pos] == '|') move();
                        break;
                    case '&':
                        if(text[_pos] == '&') move();
                        break;
                    case '!':
                    case '=':
                        if(text[_pos] == '=') move();
                        break;
                    case '>':
                        if(text[_pos] == '>' || text[_pos] == '=') move();
                        break;
                    case '<':
                        if(text[_pos] == '<' || text[_pos] == '=') move();
                        break;
                }
            }
            else if(text.substr(_pos, 2 ) == "//"){
                // line comment.
                move(2);
                for(;!linefeed();) move();
                continue;
            }else if(text.substr(_pos, 2 ) == "/*"){
                // block comment.
                move(2);
                for(;;) {
                    while (text[_pos] != '*'){
                        if(!linefeed()) move();
                    }
                    move();
                    if (text[_pos] == '/') {
                        move();
                        break;
                    }
                }
                continue;
            }else{
                // single char operator
                move();
            }
            std::string operators = text.substr(start_pos, _pos - start_pos);
            ASSERT_EXIT(operator_2_token_type.count(operators), "operator = (%s) not found", operators.c_str());
            _tokens.push_back(Token(operator_2_token_type[operators], operators, _line_number));
        }
        // string
        // discarded_char
        //      { ( ; , . [
        else if(discarded_characters_2_token_type.count(std::string(1, cp))){
            move();
            if(cp == '\"'){
                ++ start_pos;
                while(text[_pos] != '\"') move();
                std::string value = text.substr(start_pos, _pos - start_pos);
                _tokens.push_back(Token(TokenType ::STRING, value, _line_number));
                move();
            }else{
                _tokens.push_back(Token(discarded_characters_2_token_type[std::string(1, cp)], std::string(1, cp), _line_number));
            }
        }
        // others
        else if(cp == ' ' || cp == '\t' || cp == '\n' || cp == '\r'){
            // line feed
            if(linefeed()){
            } else {
                move();
            }
        }
        else{
            break;
        }
    }
    if(_line_start_pos != _pos){
        _line2line_str[_line_number ++] = _program.substr(_line_start_pos, _pos - _line_start_pos);
    }

    ASSERT_EXIT(_pos == text.size(), "error. string to be parsed:\n\n%s\n\n", text.substr(_pos).c_str());
}

void Tokenizer::print_tokens(){
    int i = 1;
    for(auto& token : _tokens){
        printf("%03d|%10s|%20s|\n", i ++, tokentype_2_string[token.token_type_].c_str(), token.values_.c_str());
    }
}
void Tokenizer::print_line_tokens(){
    for(std::pair<int, std::string> pi : _line2line_str){
        printf("%03d -> %s\n", pi.first, pi.second.c_str());
    }
}
