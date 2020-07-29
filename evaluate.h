#ifndef EVALUATE_H
#define EVALUATE_H

#include <vector>
#include <string>
#include <map>


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

    int value_type_;
    int rflag_;
    std::string tostring();
}Symbol;

typedef std::map<std::string, Symbol> Symbols;
struct Env{
    std::map<std::string, FuncProto> funcs_;
    Symbols global_;
    std::vector<Symbols> current_;
};

enum class ASTType{
    AST_BLOCK,
    // AST_STAT,
    AST_ARGS,
    AST_IF,
    AST_FOR,
    AST_ASSIGN,
    AST_EXP,
    AST_FUN,
    AST_CALL,
    AST_RETURN,
    AST_SYM,
    AST_INTEGER,
    AST_DECIMAL,
    AST_VOID,

    // op
    AST_AND,
    AST_NOT_EQUAL,
    AST_EQUAL,
    AST_LESS_EQUAL,
    AST_LARGER_EQUAL,
    AST_LESS,
    AST_LARGE,
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,


};


typedef struct AST{
    AST();
    AST(ASTType type);

    std::vector<AST*> sub_asts_;
    ASTType ast_type_;
    Symbol ast_value_;  // for simplify .

    void print(AST* ast);
    void print(int deep, AST* ast);
    AST* build(std::vector<Token*>& tokens);

    TokenType peek();
    std::string peekv();
    Token* next(TokenType token_type);
    std::string last_value();

    void args(AST* ast);
    void exp(AST* ast);
    void pass_args(AST* ast);

    void exp_elem(AST* ast);
    void exp_term(AST* ast);
    void exp_exp(AST* ast);

    void stat(AST* ast);
    void stat_if(AST *ast);
    void stat_for(AST *ast);
    void stat_exp(AST *ast);
    void stat_return(AST *ast);
    void stat_function(AST *ast);
    void stat_brace(AST *ast);
    void stat_print(AST *ast);
    void stat_input(AST *ast);

    int    eval_builtin(AST* ast);
    void   eval_function(AST* ast);
    Symbol eval_symbol(AST* ast);
    Symbol eval_exp(AST* ast);
    Symbol eval_assign(AST* ast);
    Symbol eval_if(AST* ast);
    Symbol eval_call(AST* ast);
    Symbol interpret(AST* ast);

}AST;

extern std::vector<Token *> tokens_;
extern int32_t consumed_index_;
extern std::map<ASTType, std::string> asttype_2_str_;
extern Env env_;

#endif //EVALUATE_H
