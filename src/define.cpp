#include "define.h"

std::map<std::string, TokenType> keywords_token = {
        {"function", TokenType::KEY_FUNCTION},
        {"if", TokenType::KEY_IF},
        {"while", TokenType::KEY_WHILE},
        {"return",TokenType::KEY_RETURN},
};
std::string op_keywords = std::string("=!<>+-*/|&^");
std::map<std::string, TokenType> op_char_token = {
        /*
            | -> ||
            & -> &&
            ! -> !=
            = -> ==
            > -> >= >>
            < -> <= <<
         */
        {"||", TokenType ::OP_OR},
        {"&&", TokenType ::OP_AND},
        {"|", TokenType ::OP_BIT_OR},
        {"^", TokenType ::OP_BIT_XOR},
        {"&", TokenType ::OP_BIT_AND},
        {"!=", TokenType ::OP_NOT_EQUAL},
        {"==", TokenType ::OP_EQUAL},
        {">=", TokenType ::OP_LARGER_EQUAL},
        {">", TokenType ::OP_LARGER},
        {"<=", TokenType ::OP_LESS_EQUAL},
        {"<", TokenType ::OP_LESS},
        {"<<", TokenType ::OP_SHL},
        {">>", TokenType ::OP_SHR},
        {"+", TokenType ::OP_ADD},
        {"-", TokenType ::OP_SUB},
        {"*", TokenType ::OP_MUL},
        {"/", TokenType ::OP_DIV},
        {"%", TokenType ::OP_MOD},
        {"=", TokenType ::OP_ASSIGN},
};
std::map<std::string, TokenType> easy_char_token = {
        {"{", TokenType ::LEFT_BRACE},
        {"}", TokenType ::RIGHT_BRACE},
        {"(", TokenType ::LEFT_PARENTHESIS},
        {")", TokenType ::RIGHT_PARENTHESIS},
        {";", TokenType ::SEMICOLON},
        {",", TokenType ::COMMA},
        {"'", TokenType ::QUOTATION},
        {"\"", TokenType ::DQUOTATION},
};

std::map<ASTType ,int> op_precedences = {
        {ASTType::AST_OR,1},
        {ASTType::AST_AND,2},
        {ASTType::AST_BIT_OR,3},
        {ASTType::AST_BIT_XOR,4},
        {ASTType::AST_BIT_AND,5},
        {ASTType::AST_NOT_EQUAL,6},
        {ASTType::AST_EQUAL,6},
        {ASTType::AST_LARGER_EQUAL,7},
        {ASTType::AST_LARGER,7},
        {ASTType::AST_LESS_EQUAL,7},
        {ASTType::AST_LESS,7},
        {ASTType::AST_SHL,8},
        {ASTType::AST_SHR,8},
        {ASTType::AST_ADD,9},
        {ASTType::AST_SUB,9},
        {ASTType::AST_MUL,10},
        {ASTType::AST_DIV,10},
        {ASTType::AST_MOD,10},
};







// for pretty print ---------------------------------------------------------------------------------------------------------
//                  ---------------------------------------------------------------------------------------------------------
//                  ---------------------------------------------------------------------------------------------------------
std::map<TokenType, std::string> tokentype_2_string{
        {TokenType ::SYMBOL, "symbol"},
        {TokenType ::INTEGER, "integer"},
        {TokenType ::DECIMAL, "decimal"},
        {TokenType ::STRING, "string"},
        {TokenType ::LEFT_PARENTHESIS, "("},
        {TokenType ::RIGHT_PARENTHESIS, ")"},
        {TokenType ::LEFT_BRACE, "{"},
        {TokenType ::RIGHT_BRACE, "}"},
        {TokenType ::SEMICOLON, ";"},
        {TokenType ::COMMA, ","},
        {TokenType ::KEY_WHILE, "while"},
        {TokenType ::KEY_IF, "if"},
        {TokenType ::KEY_FUNCTION, "function"},
        {TokenType ::KEY_RETURN, "return"},

        {TokenType ::OP_OR, "||"},
        {TokenType ::OP_AND, "&&"},
        {TokenType ::OP_BIT_OR, "|"},
        {TokenType ::OP_BIT_XOR, "^"},
        {TokenType ::OP_BIT_AND, "&"},
        {TokenType ::OP_NOT_EQUAL, "!="},
        {TokenType ::OP_EQUAL, "=="},
        {TokenType ::OP_LARGER_EQUAL, ">="},
        {TokenType ::OP_LARGER, ">"},
        {TokenType ::OP_LESS_EQUAL, "<="},
        {TokenType ::OP_LESS, "<"},
        {TokenType ::OP_SHL, "<<"},
        {TokenType ::OP_SHR, ">>"},
        {TokenType ::OP_ADD, "+"},
        {TokenType ::OP_SUB, "-"},
        {TokenType ::OP_MUL, "*"},
        {TokenType ::OP_DIV, "/"},
        {TokenType ::OP_MOD, "%"},

        {TokenType ::OP_ASSIGN, "="},

        {TokenType ::QUOTATION, "'"},
        {TokenType ::DQUOTATION, "\""},
        {TokenType ::END, "end"},
        {TokenType ::BUILTIN_INPUT, "input"},
        {TokenType ::BUILTIN_PRINT, "print"},
};

std::map<ASTType, std::string> asttype_2_str_ = {
        {ASTType::AST_BLOCK, "block"},
        {ASTType::AST_IF,"if"},
        {ASTType::AST_WHILE,"while"},
        {ASTType::AST_ASSIGN,"assign"},
        {ASTType::AST_EXP, "exp"},

        {ASTType::AST_ARGS, "args"},
        {ASTType::AST_FUN, "func"},
        {ASTType::AST_CALL, "call"},
        {ASTType::AST_RETURN, "return"},

        {ASTType::AST_SYM, "sym"},
        {ASTType::AST_INTEGER, "integer"},
        {ASTType::AST_DECIMAL, "decimal"},
        {ASTType::AST_STRING, "string"},

        {ASTType ::AST_OR, "||"},
        {ASTType ::AST_AND, "&&"},
        {ASTType ::AST_BIT_OR, "|"},
        {ASTType ::AST_BIT_XOR, "^"},
        {ASTType ::AST_BIT_AND, "&"},
        {ASTType ::AST_NOT_EQUAL, "!="},
        {ASTType ::AST_EQUAL, "=="},
        {ASTType ::AST_LARGER_EQUAL, ">="},
        {ASTType ::AST_LARGER, ">"},
        {ASTType ::AST_LESS_EQUAL, "<="},
        {ASTType ::AST_LESS, "<"},
        {ASTType ::AST_SHL, "<<"},
        {ASTType ::AST_SHR, ">>"},
        {ASTType ::AST_ADD, "+"},
        {ASTType ::AST_SUB, "-"},
        {ASTType ::AST_MUL, "*"},
        {ASTType ::AST_DIV, "/"},
        {ASTType ::AST_MOD, "%"},
};