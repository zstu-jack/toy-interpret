#ifndef DEFINE_H
#define DEFINE_H

#include <vector>
#include <set>
#include <map>
#include <cstdio>
#include <functional>
#include <string>

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
    FATAL,

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
    STRING,
    // for string
    QUOTATION,
    DQUOTATION,
    // builtin
    BUILTIN_INPUT,
    BUILTIN_PRINT,

    LEFT_PARENTHESIS,
    RIGHT_PARENTHESIS,
    LEFT_BRACE,
    RIGHT_BRACE,
    SEMICOLON,
    COMMA,

    KEY_WHILE,
    KEY_IF,
    KEY_FUNCTION,
    KEY_RETURN,

    OP_ASSIGN,

    OP_OR,
    OP_AND,
    OP_BIT_OR,
    OP_BIT_XOR,
    OP_BIT_AND,
    OP_NOT_EQUAL,
    OP_EQUAL,
    OP_LARGER_EQUAL,
    OP_LARGER,
    OP_LESS_EQUAL,
    OP_LESS,
    OP_SHL,
    OP_SHR,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,

    END,
};

enum class ASTType{
    AST_BLOCK,
    AST_EXP,

    AST_IF,
    AST_WHILE,
    AST_ASSIGN,

    AST_ARGS,
    AST_FUN,
    AST_CALL,
    AST_RETURN,

    AST_SYM,
    AST_INTEGER,
    AST_DECIMAL,
    AST_STRING,
    AST_VOID,

    // op, and the order represent precedences also.
    AST_OR,
    AST_AND,
    AST_BIT_OR,
    AST_BIT_XOR,
    AST_BIT_AND,
    AST_NOT_EQUAL,
    AST_EQUAL,
    AST_LARGER_EQUAL,
    AST_LARGER,
    AST_LESS_EQUAL,
    AST_LESS,
    AST_SHL,
    AST_SHR,
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_MOD,
};

extern std::map<std::string, TokenType> easy_char_token;
extern std::map<std::string, TokenType> keywords_token;

extern std::string op_keywords;
extern std::map<std::string, TokenType> op_char_token;
extern std::map<TokenType, int> op_precedences;

extern std::map<TokenType, std::string> tokentype_2_string;
extern std::map<ASTType, std::string> asttype_2_str_;
extern std::map<TokenType, ASTType > tokentype_2_asttype_;

#endif //DEFINE_H
