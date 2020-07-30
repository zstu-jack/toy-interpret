#include <iostream>

#include "parser.h"
#include "define.h"
#include "evaluate.h"

size_t _p = 0, _lp = 0, _line = 1;

Token::Token(TokenType token_type_, const std::string& values_){
    this->token_type_ = token_type_;
    this->values_ = values_;
}
Token::Token(TokenType token_type_, const std::string& values_, int line){
    this->token_type_ = token_type_;
    this->values_ = values_;
    this->lines_ = line;
}

bool Tokenizer::is_symbol_start(char c){ return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
bool Tokenizer::is_digit(char c){ return c >= '0' && c <= '9'; }
bool Tokenizer::is_symbol(char c){ return is_symbol_start(c) || is_digit(c); }
bool Tokenizer::is_operator(char c){ return op_keywords.find(c) != std::string::npos; }

void Tokenizer::incp(size_t& p){
    ASSERT_EXIT(++ p <= program_.size(), "exceed program size");
}
void Tokenizer::parse(const std::string& text){
    ASSERT_EXIT(text.size() > 0, "empty content");
    program_ = text;
    for(;;){
        if(_p == text.size()) break;
        const char& cp = text[_p];
        _lp = _p;
        if(is_symbol_start(cp)){
            while(_p < text.size() && is_symbol(text[_p])) incp(_p);
            std::string key = text.substr(_lp, _p-_lp);
            if(keywords_token.count(key)) tokens_.push_back(new Token(keywords_token[key], key, _line));
            else tokens_.push_back(new Token(TokenType ::SYMBOL, key, _line));
        }
        else if(is_digit(cp)){
            while(_p < text.size() && is_digit(text[_p])) incp(_p);
            if(text[_p] == '.') incp(_p);
            while(_p < text.size() && is_digit(text[_p])) incp(_p);
            std::string key = text.substr(_lp, _p-_lp);
            if(key.find('.') != std::string::npos) tokens_.push_back(new Token(TokenType ::DECIMAL, key, _line));
            else tokens_.push_back(new Token(TokenType ::INTEGER, key, _line));
        }
        else if(is_operator(cp)){
            incp(_p);
            if(text[_p] == '=' || text[_p] == '|' || text[_p] == '&') {
                incp(_p);
                std::string key = text.substr(_lp, _p - _lp);
            }
            std::string key = text.substr(_lp, _p-_lp);
            ASSERT_EXIT(op_char_token.count(key), "operator = (%s) not found", key.c_str());
            tokens_.push_back(new Token(op_char_token[key], key, _line));
        }
        else if(easy_char_token.count(std::string(1, cp))){
            incp(_p);
            tokens_.push_back(new Token(easy_char_token[std::string(1, cp)], std::string(1, cp), _line));
        }
        else if(cp == ' ' || cp == '\t' || cp == '\n' || cp == '\r'){
            incp(_p);
            if(cp == '\r'){
                if(text[_p] == '\n'){
                    _line ++;
                    incp(_p);
                }
            }
            if(cp == '\n'){
                ++ _line;
            }
        }
        else{
            break;
        }
        // ' "
    }
    ASSERT_EXIT(_p == text.size(), "error. string to be parsed:[%s]", text.substr(_p).c_str());
}

void Tokenizer::print(){
    int i = 0;
    for(auto& token : tokens_){
        printf("%03d|%5d|%20s|\n", i ++, token->token_type_, token->values_.c_str());
    }

}
