#ifndef DEFINE_H
#define DEFINE_H

#include <vector>
#include <set>
#include <map>
#include <cstdio>
#include <functional>

#define ASSERT_EXIT(COND, X, ...)\
    if(!(COND)) \
    { \
        fprintf(stderr, "[filename:%s, function:%s, line:%d]\n", __FILE__, __func__, __LINE__); \
        fprintf(stderr, "( %s ) assert failed\n" X, (#COND), ##__VA_ARGS__); \
        fflush(stderr);\
        exit(-1);\
    }

enum class ResultID{
    SUCCESS = 0,
    FATAL = 1,

};

enum class SymbolType{
    FUNCTION = 0,
    INTEGER,
    DECIMAL,
    STRING,
};

enum class TokenType{
    SYMBOL = 0,
    INTEGER,
    DECIMAL,
    LEFT_PARENTHESIS,
    RIGHT_PARENTHESIS,
    LEFT_BRACE,
    RIGHT_BRACE,
    SEMICOLON,
    COMMA,

    KEY_FOR,
    KEY_IF,
    KEY_FUNCTION,
    KEY_RETURN,

    OP_ASSIGN,
    OP_NOT_EQUAL,
    OP_EQUAL,
    OP_LESS_EQUAL,
    OP_LARGER_EQUAL,
    OP_LESS,
    OP_LARGER,
    OP_BIT_AND,
    OP_BIT_OR,
    OP_AND,
    OP_OR,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,

    QUOTATION,
    DQUOTATION,

    END,

    // builtin
    BUILTIN_INPUT,
    BUILTIN_PRINT,
};

extern std::map<std::string, TokenType> keywords_token;
extern std::string op_keywords;
extern std::map<char, TokenType> easy_char_token;
extern std::map<std::string, TokenType> op_char_token;

extern std::map<TokenType, std::string> tokentype_2_string;

#endif //DEFINE_H
