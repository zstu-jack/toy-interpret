#ifndef EVALUATE_H
#define EVALUATE_H

#include <vector>
#include <string>
#include <map>

#include "define.h"

struct AST;
struct Token;
enum class TokenType;

typedef struct FuncProto{
    std::vector<std::string> args_symbol_;
    AST* ast;
}FuncProto;

typedef struct Symbol{
    Symbol();

    std::string str;
    int num;
    double dec;
    FuncProto func;
    ASTType value_type_;

    int return_flag_;
    std::string tostring();
}Symbol;

typedef std::map<std::string, Symbol> Symbols;
struct Env{
    std::map<std::string, FuncProto> funcs_;
    Symbols global_;
    std::vector<Symbols> current_;
};


typedef struct AST{
    AST();
    AST(ASTType type);

    std::vector<AST*> sub_asts_;
    ASTType ast_type_;
    Symbol ast_value_;

    void print(AST* ast);
    void print(int deep, AST* ast);

    TokenType peek_type();
    std::string peek_value();
    Token* next(TokenType token_type);
    std::string last_value();

    void args(AST* ast);
    void pass_args(AST* ast);

    void exp_elem(AST* ast);
    void exp_term(AST* ast);
    void exp_exp(AST* ast);
    void exp(AST* ast);

    void stat(AST* ast);    // stats -> stat | stat {stats}
    void stat_if(AST *ast);
    void stat_while(AST *ast);
    void stat_exp(AST *ast);
    void stat_return(AST *ast);
    void stat_function(AST *ast);
    void stat_brace(AST *ast);
    void stat_print(AST *ast);
    void stat_input(AST *ast);
    AST* build(std::vector<Token*>& tokens);

    int is_builtin(AST* ast);
    Symbol eval_builtin(AST* ast);
    Symbol eval_function(AST* ast);
    Symbol eval_symbol(AST* ast);
    Symbol eval_exp(AST* ast);          // TODO
    Symbol eval_assign(AST* ast);
    Symbol eval_if(AST* ast);
    Symbol eval_while(AST* ast);
    Symbol eval_call(AST* ast);
    Symbol interpret(AST* ast);

}AST;

extern std::vector<Token *> tokens_;
extern size_t consumed_index_;
extern Env env_;

#endif //EVALUATE_H
