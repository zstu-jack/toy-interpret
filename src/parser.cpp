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

bool Tokenizer::is_symbol_start(char c){ return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '.'; }
bool Tokenizer::is_digit(char c){ return c >= '0' && c <= '9'; }
bool Tokenizer::is_symbol(char c){ return is_symbol_start(c) || is_digit(c); }
bool Tokenizer::is_operator(char c){ return op_keywords.find(c) != std::string::npos; }
bool Tokenizer::pass_linefeed(){
    if(program_[_p] == '\n'){
        line2line_str_[_line ++] = program_.substr(_linep_start,  _p - _linep_start);
        incp(_p);
        _linep_start = _p;
        return true;
    }
    if(program_[_p] == '\r' && program_[_p+1] == '\n'){
        line2line_str_[_line ++] = program_.substr(_linep_start,  _p - _linep_start);
        incp(_p, 2);
        _linep_start = _p;
        return true;
    }
    return false;
}

Tokenizer::Tokenizer(){
    _p = 0;
    _lp = 0;
    _line = 1;
    _linep_start = 0;
}

void Tokenizer::incp(size_t& p, size_t offset){
    for(size_t i = 0; i < offset; i ++) {
        ASSERT_EXIT(++p <= program_.size(), "exceed program size");
    }
}
void Tokenizer::parse(const std::string& text){
    ASSERT_EXIT(text.size() > 0, "empty content");
    program_ = text;
    for(;;){
        if(_p == text.size()) break;
        const char& cp = text[_p];
        _lp = _p;

        // symbol or keyword
        if(is_symbol_start(cp)){
            while(_p < text.size() && is_symbol(text[_p])) incp(_p);
            std::string key = text.substr(_lp, _p-_lp);
            if(keywords_token.count(key)) tokens_.push_back(new Token(keywords_token[key], key, _line));
            else tokens_.push_back(new Token(TokenType ::SYMBOL, key, _line));
        }
        // decimal or integer
        else if(is_digit(cp)){
            while(_p < text.size() && is_digit(text[_p])) incp(_p);
            if(text[_p] == '.') incp(_p);
            while(_p < text.size() && is_digit(text[_p])) incp(_p);
            std::string key = text.substr(_lp, _p-_lp);
            if(key.find('.') != std::string::npos) tokens_.push_back(new Token(TokenType ::DECIMAL, key, _line));
            else tokens_.push_back(new Token(TokenType ::INTEGER, key, _line));
        }
        else if(is_operator(cp)){
            // operator: char start with `| & ! = > <` may have next char that represent op together.
            //   | -> ||
            //   & -> &&
            //   ! -> !=
            //   = -> ==
            //   > -> >= >>
            //   < -> <= <<

            // two-char operator
            static std::string start_str = "|&!=><";
            if(start_str.find(text[_p]) != std::string::npos) {
                incp(_p);
                switch (text[_p]){
                    case '|':
                        if(text[_p] == '|') incp(_p);
                        break;
                    case '&':
                        if(text[_p] == '&') incp(_p);
                        break;
                    case '!':
                    case '=':
                        if(text[_p] == '=') incp(_p);
                        break;
                    case '>':
                        if(text[_p] == '>' || text[_p] == '=') incp(_p);
                        break;
                    case '<':
                        if(text[_p] == '<' || text[_p] == '=') incp(_p);
                        break;
                }
            }
            else if(text[_p] == '/' && text[_p+1] == '/'){
                // line comment.
                incp(_p, 2);
                for(;!pass_linefeed();) incp(_p);
                continue;

            }else if(text[_p] == '/' && text[_p+1] == '*'){
                // block comment.
                incp(_p, 2);
                for(;;) {
                    while (text[_p] != '*'){
                        if(!pass_linefeed()) incp(_p);
                    }
                    incp(_p);
                    if (text[_p] == '/') {
                        incp(_p);
                        break;
                    }
                }
                continue;
            }else{
                // single char operator
                incp(_p);
            }
            std::string key = text.substr(_lp, _p-_lp);
            ASSERT_EXIT(op_char_token.count(key), "operator = (%s) not found", key.c_str());
            tokens_.push_back(new Token(op_char_token[key], key, _line));
        }
        else if(easy_char_token.count(std::string(1, cp))){
            // string or `{ ( ; ,`
            incp(_p);
            if(cp == '\"'){
                ++ _lp;
                while(text[_p] != '\"') incp(_p);
                std::string value = text.substr(_lp, _p-_lp);
                tokens_.push_back(new Token(TokenType ::STRING, value, _line));
                incp(_p);
            }else{
                tokens_.push_back(new Token(easy_char_token[std::string(1, cp)], std::string(1, cp), _line));
            }
        }
        else if(cp == ' ' || cp == '\t' || cp == '\n' || cp == '\r'){
            // line feed
            if(!pass_linefeed()){
                incp(_p);
            }
        }
        else{
            break;
        }
        // ' "
    }
    if(_linep_start != _p){
        line2line_str_[_line ++] = program_.substr(_linep_start,  _p - _linep_start);
    }

    ASSERT_EXIT(_p == text.size(), "error. string to be parsed:[%s]", text.substr(_p).c_str());
}

void Tokenizer::print(){
    int i = 0;
    for(auto& token : tokens_){
        printf("%03d|%5d|%20s|\n", i ++, token->token_type_, token->values_.c_str());
    }
}
void Tokenizer::print_lines(){
    for(std::pair<int, std::string> pi : line2line_str_){
        printf("%03d -> %s\n", pi.first, pi.second.c_str());
    }
}
