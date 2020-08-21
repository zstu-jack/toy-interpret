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
        {"[", TokenType ::LEFT_BRACKET},
        {"]", TokenType ::RIGHT_BRACKET},
        {"{", TokenType ::LEFT_BRACE},
        {"}", TokenType ::RIGHT_BRACE},
        {"(", TokenType ::LEFT_PARENTHESIS},
        {")", TokenType ::RIGHT_PARENTHESIS},
        {";", TokenType ::SEMICOLON},
        {",", TokenType ::COMMA},
        {".", TokenType ::DOT},
        {"'", TokenType ::QUOTATION},
        {"\"", TokenType ::DQUOTATION},
};

std::map<TokenType ,int> op_precedences = {
        {TokenType::OP_OR,1},
        {TokenType::OP_AND,2},
        {TokenType::OP_BIT_OR,3},
        {TokenType::OP_BIT_XOR,4},
        {TokenType::OP_BIT_AND,5},
        {TokenType::OP_NOT_EQUAL,6},
        {TokenType::OP_EQUAL,6},
        {TokenType::OP_LARGER_EQUAL,7},
        {TokenType::OP_LARGER,7},
        {TokenType::OP_LESS_EQUAL,7},
        {TokenType::OP_LESS,7},
        {TokenType::OP_SHL,8},
        {TokenType::OP_SHR,8},
        {TokenType::OP_ADD,9},
        {TokenType::OP_SUB,9},
        {TokenType::OP_MUL,10},
        {TokenType::OP_DIV,10},
        {TokenType::OP_MOD,10},
};

std::map<TokenType , ASTType> tokentype_2_asttype_ = {
{TokenType ::OP_OR, ASTType ::AST_OR},
{TokenType ::OP_AND, ASTType ::AST_AND},
{TokenType ::OP_BIT_OR, ASTType ::AST_BIT_OR},
{TokenType ::OP_BIT_XOR, ASTType ::AST_BIT_XOR},
{TokenType ::OP_BIT_AND, ASTType ::AST_BIT_AND},
{TokenType ::OP_NOT_EQUAL, ASTType ::AST_NOT_EQUAL},
{TokenType ::OP_EQUAL, ASTType ::AST_EQUAL},
{TokenType ::OP_LARGER_EQUAL, ASTType ::AST_LARGER_EQUAL},
{TokenType ::OP_LARGER, ASTType ::AST_LARGER},
{TokenType ::OP_LESS_EQUAL, ASTType ::AST_LESS_EQUAL},
{TokenType ::OP_LESS, ASTType ::AST_LESS},
{TokenType ::OP_SHL, ASTType ::AST_SHL},
{TokenType ::OP_SHR, ASTType ::AST_SHR},
{TokenType ::OP_ADD, ASTType ::AST_ADD},
{TokenType ::OP_SUB, ASTType ::AST_SUB},
{TokenType ::OP_MUL, ASTType ::AST_MUL},
{TokenType ::OP_DIV, ASTType ::AST_DIV},
{TokenType ::OP_MOD, ASTType ::AST_MOD}
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
        {TokenType ::LEFT_BRACKET, "["},
        {TokenType ::RIGHT_BRACKET, "]"},
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
        {ASTType::AST_INITLIST, "initlist"},

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