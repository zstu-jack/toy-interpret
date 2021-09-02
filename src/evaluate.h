#ifndef EVALUATE_H
#define EVALUATE_H

#include <vector>
#include <string>
#include <map>

#include "parser.h"
#include "define.h"

struct AST;
struct Object;
struct Symbol;
enum class TokenType;

typedef struct FuncProto{
    std::vector<std::string> args_symbol_;
    AST* ast;
}FuncProto;

typedef struct Symbol{
    Symbol();
    ~Symbol();
    Symbol(const Symbol& symbol);
    Symbol(Symbol&& symbol);
    Symbol& operator=(const Symbol& symbol);
    Symbol& operator=(Symbol&& symbol);
    void FieldCopy(const Symbol& symbol);

    ASTType type_;

    long long num;
    double dec;
    std::string str;
    Object* object;
    FuncProto func;

    int return_flag_;
    std::string tostring();

    Symbol& operator+(const Symbol& symbol);
    Symbol& operator-(const Symbol& symbol);
    Symbol& operator*(const Symbol& symbol);
    Symbol& operator/(const Symbol& symbol);
    Symbol& operator%(const Symbol& symbol);
    Symbol& operator||(const Symbol& symbol);
    Symbol& operator&&(const Symbol& symbol);
    Symbol& operator|(const Symbol& symbol);
    Symbol& operator&(const Symbol& symbol);
    Symbol& operator^(const Symbol& symbol);
    Symbol& operator!=(const Symbol& symbol);
    Symbol& operator==(const Symbol& symbol);
    Symbol& operator>(const Symbol& symbol);
    Symbol& operator>=(const Symbol& symbol);
    Symbol& operator<(const Symbol& symbol);
    Symbol& operator<=(const Symbol& symbol);
    Symbol& operator<<(const Symbol& symbol);
    Symbol& operator>>(const Symbol& symbol);
}Symbol;
extern std::map<ASTType , std::function<Symbol&(Symbol*, const Symbol&)> > arith_callback_;

typedef struct Object{
    std::map<int, Symbol> obj_;
    std::map<std::string, int> idx_;
}Object;

typedef std::map<std::string, Symbol> Symbols;
struct Env{
    std::map<std::string, FuncProto> funcs_;
    Symbols global_;
    std::vector<Symbols> current_;
};

typedef struct AST{
    AST();
    AST(ASTType type);
    AST(ASTType type, const std::string& str);

    std::vector<AST*> sub_asts_;
    ASTType ast_type_;
    Symbol ast_value_;

    void print(AST* ast);
    void print(int deep, AST* ast);
    void print_symbol(std::string name, const Symbol& symbol);

    TokenType peek_type();
    std::string peek_value();
    void check_next(TokenType token_type);
    Token next(TokenType token_type);
    std::string last_value();

    /**
     * stats    -> stat | stat {stats}
     * stat     ->  if
     *              while
     *              function
     *              symbol (function call / expression / ...)
     *              return
     *              stats ( {} )
     *  if      -> (exp) { stats }
     *  while   -> (exp) { stats }
     *  function-> (args) { stats }
     *  return  -> exp_semicolon / ;
     *  exp_elem-> (), {}, integer, decimal, string, symbol
     */
    AST* stat();
    AST* stat_block();
    AST* stat_if();
    AST* stat_while();
    AST* stat_function();
    AST* stat_function_args();
    AST* stat_exp_elem();
    AST* stat_exp(int pre = -1);
    AST* stat_exp_semicolon();
    void stat_parameters_passed(AST* ast, TokenType end);
    AST* stat_return();

    int is_builtin(AST* ast);
    Symbol eval_builtin(AST* ast);
    Symbol eval_function(AST* ast);
    Symbol eval_symbol(AST* ast);
    Symbol eval_exp(AST* ast);
    Symbol eval_assign(AST* ast);
    Symbol eval_if(AST* ast);
    Symbol eval_while(AST* ast);
    Symbol eval_call(AST* ast);
    Symbol interpret(AST* ast);

    AST* build(Tokenizer tokenizer);
    static Tokenizer tokenizer_;
    static std::vector<Token> tokens_;
    static int32_t token_index_;
    static Env env_;
}AST;

#endif //EVALUATE_H
