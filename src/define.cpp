#include "define.h"

std::map<std::string, TokenType> keywords_token = {
        {"function", TokenType::KEY_FUNCTION},
        {"if", TokenType::KEY_IF},
        {"for", TokenType::KEY_FOR},
        {"return",TokenType::KEY_RETURN},
};

std::string op_keywords = std::string("=!<>+-*/|&");

std::map<char, TokenType> easy_char_token = {
        {'{', TokenType ::LEFT_BRACE},
        {'}', TokenType ::RIGHT_BRACE},
        {'(', TokenType ::LEFT_PARENTHESIS},
        {')', TokenType ::RIGHT_PARENTHESIS},
        {';', TokenType ::SEMICOLON},
        {',', TokenType ::COMMA},
};

std::map<std::string, TokenType> op_char_token = {
        {"=", TokenType ::OP_ASSIGN},
        {"!=", TokenType ::OP_NOT_EQUAL},
        {"==", TokenType ::OP_EQUAL},
        {"<=", TokenType ::OP_LESS_EQUAL},
        {"<", TokenType ::OP_LESS},
        {">", TokenType ::OP_LARGER},
        {">=", TokenType ::OP_LARGER_EQUAL},
        {"&", TokenType ::OP_BIT_AND},
        {"|", TokenType ::OP_BIT_OR},
        {"&&", TokenType ::OP_AND},
        {"||", TokenType ::OP_OR},
        {"+", TokenType ::OP_ADD},
        {"-", TokenType ::OP_SUB},
        {"*", TokenType ::OP_MUL},
        {"/", TokenType ::OP_DIV},
};

std::map<TokenType, std::string> tokentype_2_string{
        {TokenType ::SYMBOL, "symbol"},
        {TokenType ::INTEGER, "integer"},
        {TokenType ::DECIMAL, "decimal"},
        {TokenType ::LEFT_PARENTHESIS, "("},
        {TokenType ::RIGHT_PARENTHESIS, ")"},
        {TokenType ::LEFT_BRACE, "{"},
        {TokenType ::RIGHT_BRACE, "}"},
        {TokenType ::SEMICOLON, ";"},
        {TokenType ::COMMA, ","},
        {TokenType ::KEY_FOR, "for"},
        {TokenType ::KEY_IF, "if"},
        {TokenType ::KEY_FUNCTION, "function"},
        {TokenType ::KEY_RETURN, "return"},
        {TokenType ::OP_ASSIGN, "="},
        {TokenType ::OP_NOT_EQUAL, "!="},
        {TokenType ::OP_EQUAL, "=="},
        {TokenType ::OP_LESS_EQUAL, "<="},
        {TokenType ::OP_LARGER_EQUAL, ">="},
        {TokenType ::OP_LESS, "<"},
        {TokenType ::OP_LARGER, ">"},
        {TokenType ::OP_BIT_AND, "&"},
        {TokenType ::OP_BIT_OR, "|"},
        {TokenType ::OP_AND, "&&"},
        {TokenType ::OP_OR, "||"},
        {TokenType ::OP_ADD, "+"},
        {TokenType ::OP_SUB, "-"},
        {TokenType ::OP_MUL, "*"},
        {TokenType ::OP_DIV, "/"},
        {TokenType ::QUOTATION, "'"},
        {TokenType ::DQUOTATION, "\"\""},
        {TokenType ::END, "end"},
        {TokenType ::BUILTIN_INPUT, "input"},
        {TokenType ::BUILTIN_PRINT, "print"},
};