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

bool Tokenizer::is_symbol_start(char c){ return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
bool Tokenizer::is_digit(char c){ return c >= '0' && c <= '9'; }
bool Tokenizer::is_symbol(char c){ return is_symbol_start(c) || is_digit(c); }
bool Tokenizer::is_operator(char c){ return keywords_operator.find(c) != std::string::npos; }
bool Tokenizer::eat_linefeed(){
    if(program_[_parsing_pos] == '\n'){
        line2line_str_[_line_number ++] = program_.substr(_line_start_pos, _parsing_pos - _line_start_pos);
        add_parsing_pos(_parsing_pos);
        _line_start_pos = _parsing_pos;
        return true;
    }
    if(program_[_parsing_pos] == '\r' && program_[_parsing_pos + 1] == '\n'){
        line2line_str_[_line_number ++] = program_.substr(_line_start_pos, _parsing_pos - _line_start_pos);
        add_parsing_pos(_parsing_pos, 2);
        _line_start_pos = _parsing_pos;
        return true;
    }
    return false;
}

Tokenizer::Tokenizer(){
    _parsing_pos = 0;
    _l_parsing_pos = 0;
    _line_number = 1;
    _line_start_pos = 0;
}

void Tokenizer::add_parsing_pos(size_t& p, size_t offset){
    p += offset;
    ASSERT_EXIT(p <= program_.size(), "exceed program size");

}
void Tokenizer::parse(const std::string& text){
    ASSERT_EXIT(text.size() > 0, "empty content");
    program_ = text;
    for(;;){
        if(_parsing_pos == text.size()) break;
        const char& cp = text[_parsing_pos];
        _l_parsing_pos = _parsing_pos;

        // symbol or keyword
        if(is_symbol_start(cp)){
            while(_parsing_pos < text.size() && is_symbol(text[_parsing_pos])) add_parsing_pos(_parsing_pos);
            std::string key = text.substr(_l_parsing_pos, _parsing_pos - _l_parsing_pos);
            if(keywords_2_token_type.count(key)) tokens_.push_back(new Token(keywords_2_token_type[key], key, _line_number));
            else tokens_.push_back(new Token(TokenType ::SYMBOL, key, _line_number));
        }
        // decimal or integer
        else if(is_digit(cp)){
            while(_parsing_pos < text.size() && is_digit(text[_parsing_pos])) add_parsing_pos(_parsing_pos);
            if(text[_parsing_pos] == '.') add_parsing_pos(_parsing_pos);
            while(_parsing_pos < text.size() && is_digit(text[_parsing_pos])) add_parsing_pos(_parsing_pos);
            std::string key = text.substr(_l_parsing_pos, _parsing_pos - _l_parsing_pos);
            if(key.find('.') != std::string::npos) tokens_.push_back(new Token(TokenType ::DECIMAL, key, _line_number));
            else tokens_.push_back(new Token(TokenType ::INTEGER, key, _line_number));
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
            if(keywords_operator_start.find(text[_parsing_pos]) != std::string::npos) {
                add_parsing_pos(_parsing_pos);
                switch (text[_parsing_pos]){
                    case '|':
                        if(text[_parsing_pos] == '|') add_parsing_pos(_parsing_pos);
                        break;
                    case '&':
                        if(text[_parsing_pos] == '&') add_parsing_pos(_parsing_pos);
                        break;
                    case '!':
                    case '=':
                        if(text[_parsing_pos] == '=') add_parsing_pos(_parsing_pos);
                        break;
                    case '>':
                        if(text[_parsing_pos] == '>' || text[_parsing_pos] == '=') add_parsing_pos(_parsing_pos);
                        break;
                    case '<':
                        if(text[_parsing_pos] == '<' || text[_parsing_pos] == '=') add_parsing_pos(_parsing_pos);
                        break;
                }
            }
            else if(text[_parsing_pos] == '/' && text[_parsing_pos + 1] == '/'){
                // line comment.
                add_parsing_pos(_parsing_pos, 2);
                for(;!eat_linefeed();) add_parsing_pos(_parsing_pos);
                continue;
            }else if(text[_parsing_pos] == '/' && text[_parsing_pos + 1] == '*'){
                // block comment.
                add_parsing_pos(_parsing_pos, 2);
                for(;;) {
                    while (text[_parsing_pos] != '*'){
                        if(!eat_linefeed()) add_parsing_pos(_parsing_pos);
                    }
                    add_parsing_pos(_parsing_pos);
                    if (text[_parsing_pos] == '/') {
                        add_parsing_pos(_parsing_pos);
                        break;
                    }
                }
                continue;
            }else{
                // single char operator
                add_parsing_pos(_parsing_pos);
            }
            std::string key = text.substr(_l_parsing_pos, _parsing_pos - _l_parsing_pos);
            ASSERT_EXIT(operator_2_token_type.count(key), "operator = (%s) not found", key.c_str());
            tokens_.push_back(new Token(operator_2_token_type[key], key, _line_number));
        }
        else if(discarded_characters_2_token_type.count(std::string(1, cp))){
            // string or `{ ( ; , . [`
            add_parsing_pos(_parsing_pos);
            if(cp == '\"'){
                ++ _l_parsing_pos;
                while(text[_parsing_pos] != '\"') add_parsing_pos(_parsing_pos);
                std::string value = text.substr(_l_parsing_pos, _parsing_pos - _l_parsing_pos);
                tokens_.push_back(new Token(TokenType ::STRING, value, _line_number));
                add_parsing_pos(_parsing_pos);
            }else{
                tokens_.push_back(new Token(discarded_characters_2_token_type[std::string(1, cp)], std::string(1, cp), _line_number));
            }
        }
        else if(cp == ' ' || cp == '\t' || cp == '\n' || cp == '\r'){
            // line feed
            if(!eat_linefeed()){
                add_parsing_pos(_parsing_pos);
            }
        }
        else{
            break;
        }
    }
    if(_line_start_pos != _parsing_pos){
        line2line_str_[_line_number ++] = program_.substr(_line_start_pos, _parsing_pos - _line_start_pos);
    }

    ASSERT_EXIT(_parsing_pos == text.size(), "error. string to be parsed:\n\n%s\n\n", text.substr(_parsing_pos).c_str());
}

void Tokenizer::print(){
    int i = 1;
    for(auto& token : tokens_){
        printf("%03d|%10s|%20s|\n", i ++, tokentype_2_string[token->token_type_].c_str(), token->values_.c_str());
    }
}
void Tokenizer::print_lines(){
    for(std::pair<int, std::string> pi : line2line_str_){
        printf("%03d -> %s\n", pi.first, pi.second.c_str());
    }
}
