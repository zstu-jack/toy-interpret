#include <iostream>

#include "parser.h"
#include "define.h"
#include "evaluate.h"

size_t _p = 0, _lp = 0;

Token::Token(int token_type_, const std::string& values_){ this->token_type_ = token_type_; this->values_ = values_; }

bool Tokenizer::is_symbol_start(char c){ return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
bool Tokenizer::is_digit(char c){ return c >= '0' && c <= '9'; }
bool Tokenizer::is_symbol(char c){ return is_symbol_start(c) || is_digit(c); }
bool Tokenizer::is_operator(char c){ return op_keywords.find(c) != std::string::npos; }

void Tokenizer::parse(const std::string& text){
    ASSERT_EXIT(text.size() > 0, "empty content");

    for(;;){
        if(_p == text.size()) break;
        const char& cp = text[_p];
        _lp = _p;
        if(is_symbol_start(cp)){
            while(_p < text.size() && is_symbol(text[_p])) _p ++;
            std::string key = text.substr(_lp, _p-_lp);
            if(keywords_token.count(key)) tokens_.push_back(new Token(static_cast<int >(keywords_token[key]), key));
            else tokens_.push_back(new Token(static_cast<int >(TokenType ::SYMBOL), key));
        }
        else if(is_digit(cp)){
            while(_p < text.size() && is_digit(text[_p])) _p ++;
            if(text[_p] == '.') _p ++;
            while(_p < text.size() && is_digit(text[_p])) _p ++;
            std::string key = text.substr(_lp, _p-_lp);
            if(key.find('.') != std::string::npos) tokens_.push_back(new Token(static_cast<int >(TokenType ::DECIMAL), key));
            else tokens_.push_back(new Token(static_cast<int >(TokenType ::INTEGER), key));
        }
        else if(is_operator(cp)){
            _p ++;
            if(text[_p] == '=') _p ++;
            else if((_p < text.size() && cp == '|') || (cp == '&' && text[_p] == cp)) _p ++;
            std::string key = text.substr(_lp, _p-_lp);
            tokens_.push_back(new Token((int)op_char_token[key], key));
        }
        else if(easy_char_token.count(cp)){
            _p ++;
            std::string cstr(1, cp);
            tokens_.push_back(new Token(static_cast<int >(easy_char_token[cp]), cstr));
        }
        else if(cp == ' ' || cp == '\t' || cp == '\n' || cp == '\r'){
            _p ++;
        }
        else{
            break;
        }
        // ' "
    }
    std::cout << _p << std::endl;
    // entire program for now.
    ASSERT_EXIT(_p == text.size(), "error. left string to be parsed:[%s]", text.substr(_p).c_str());
}

void Tokenizer::print(){
    int i = 0;
    for(auto& token : tokens_){
        printf("%03d|%5d|%20s|\n", i ++, token->token_type_, token->values_.c_str());
    }

}
