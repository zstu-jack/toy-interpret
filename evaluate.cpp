#include <bits/stdc++.h>

#include "evaluate.h"
#include "utils.h"
#include "define.h"
#include "parser.h"

std::vector<Token *> tokens_;
int32_t consumed_index_;
Env env_;

std::map<ASTType, std::string> asttype_2_str_ = {
        {ASTType::AST_BLOCK, "block"},
        {ASTType::AST_ARGS, "args"},
        {ASTType::AST_IF,"if"},
        {ASTType::AST_FOR,"for"},
        {ASTType::AST_ASSIGN,"assign"},
        {ASTType::AST_EXP, "exp"},
        {ASTType::AST_FUN, "func"},
        {ASTType::AST_CALL, "call"},
        {ASTType::AST_RETURN, "return"},
        {ASTType::AST_SYM, "sym"},

        {ASTType::AST_INTEGER, "integer"},
        {ASTType::AST_DECIMAL, "decimal"},
        {ASTType::AST_AND, "&&"},
        {ASTType::AST_NOT_EQUAL, "<="},
        {ASTType::AST_EQUAL, "=="},
        {ASTType::AST_LESS_EQUAL, "<="},
        {ASTType::AST_LARGER_EQUAL, ">="},
        {ASTType::AST_LESS, "<"},
        {ASTType::AST_LARGE, ">"},
        {ASTType::AST_ADD, "+"},
        {ASTType::AST_SUB, "-"},
        {ASTType::AST_MUL, "*"},
        {ASTType::AST_DIV, "/"},
};

Symbol::Symbol(){
    num = 0;
    dec = 0;
    str = "";
    rflag_ = 0;
}

std::string Symbol::tostring(){

}

AST::AST(ASTType type){
    this->ast_type_ = type;
}
AST::AST(){}

TokenType AST::peek(){
    if(consumed_index_ == tokens_.size()){
        return TokenType ::END;
    }
    return (TokenType)tokens_[consumed_index_]->token_type_;
}
std::string AST::peekv(){
    return tokens_[consumed_index_]->values_;
}
std::string AST::last_value(){
    return tokens_[consumed_index_-1]->values_;
}
Token* AST::next(TokenType token_type){
    ASSERT_EXIT(tokens_[consumed_index_]->token_type_ == static_cast<int>(token_type),"consumed_index=%d, expected 【%s】 but got 【%s】",
            consumed_index_,tokentype_2_string[(TokenType)token_type].c_str(), tokentype_2_string[(TokenType)tokens_[consumed_index_]->token_type_].c_str());
    return tokens_[consumed_index_ ++];
}

void AST::exp_elem(AST* ast){
    if(peek() == TokenType::LEFT_PARENTHESIS){
        next(TokenType::LEFT_PARENTHESIS);
        exp(ast);
        next(TokenType::RIGHT_PARENTHESIS);
    } else if(peek() == TokenType::INTEGER){
        ast->ast_type_ = ASTType ::AST_INTEGER;
        ast->ast_value_.num = std::stoi(peekv());
        next(TokenType::INTEGER);
    } else if(peek() == TokenType::DECIMAL){
        ast->ast_type_ = ASTType ::AST_DECIMAL;
        ast->ast_value_.dec = std::stod(peekv());
        next(TokenType::DECIMAL);
    } else if(peek() == TokenType::SYMBOL){
        ast->ast_type_ = ASTType ::AST_SYM;
        ast->ast_value_.str = peekv();
        auto symbolv = next(TokenType::SYMBOL)->values_;
        if(peek() == TokenType::LEFT_PARENTHESIS){
            ast->ast_type_ = ASTType ::AST_CALL;
            ast->ast_value_.str = symbolv;
            next(TokenType::LEFT_PARENTHESIS);
            pass_args(ast);
            next(TokenType::RIGHT_PARENTHESIS);
        }
    }else{
        ASSERT_EXIT(false, "unexpected token type %d\n", static_cast<int >(peek()));
    }
}

void AST::exp_term(AST* ast){
    ast->sub_asts_.push_back(new AST(ASTType::AST_EXP));
    exp_elem(ast->sub_asts_.back());
    if (peek() == TokenType::OP_MUL || peek() == TokenType::OP_DIV){
        if(peek() == TokenType::OP_MUL){
            ast->ast_type_ = ASTType ::AST_MUL;
            next(TokenType::OP_MUL);
        }else{
            ast->ast_type_ = ASTType ::AST_DIV;
            next(TokenType::OP_DIV);
        }
        ast->sub_asts_.push_back(new AST(ASTType::AST_EXP));
        exp_term(ast->sub_asts_.back());
    }
}

void AST::exp_exp(AST* ast){
    ast->sub_asts_.push_back(new AST(ASTType::AST_EXP));
    exp_term(ast->sub_asts_.back());
    if (peek() == TokenType::OP_ADD || peek() == TokenType::OP_SUB){
        if(peek() == TokenType::OP_ADD){
            ast->ast_type_ = ASTType ::AST_ADD;
            next(TokenType::OP_ADD);
        }else{
            ast->ast_type_ = ASTType ::AST_SUB;
            next(TokenType::OP_SUB);
        }
        ast->sub_asts_.push_back(new AST(ASTType::AST_EXP));
        exp_exp(ast->sub_asts_.back());
    }
}

void AST::exp(AST* ast){
    // TODO: Use Top Down Operator Precedence !
    if(peek() == TokenType::LEFT_PARENTHESIS){

    }
    ast->sub_asts_.push_back(new AST(ASTType::AST_EXP));    // only one node in exp.
    exp_exp(ast->sub_asts_.back());

    if (peek() == TokenType::OP_LARGER) {
        next(TokenType::OP_LARGER);
        ast->ast_type_ = ASTType ::AST_LARGE;
        ast->sub_asts_.push_back(new AST(ASTType::AST_EXP));
        exp_exp(ast->sub_asts_.back());
    }
    else if (peek() == TokenType::OP_LESS) {
        next(TokenType::OP_LESS);
        ast->ast_type_ = ASTType ::AST_LESS;
        ast->sub_asts_.push_back(new AST(ASTType::AST_EXP));
        exp_exp(ast->sub_asts_.back());
    }
    else if (peek() == TokenType::OP_LARGER_EQUAL) {
        next(TokenType::OP_LARGER_EQUAL);
        ast->ast_type_ = ASTType ::AST_LARGE;
        ast->sub_asts_.push_back(new AST(ASTType::AST_EXP));
        exp_exp(ast->sub_asts_.back());
    }
    else if (peek() == TokenType::OP_LESS_EQUAL) {
        next(TokenType::OP_LESS_EQUAL);
        ast->ast_type_ = ASTType ::AST_LESS_EQUAL;
        ast->sub_asts_.push_back(new AST(ASTType::AST_EXP));
        exp_exp(ast->sub_asts_.back());
    }
    else if (peek() == TokenType::OP_EQUAL) {
        next(TokenType::OP_EQUAL);
        ast->ast_type_ = ASTType ::AST_EQUAL;
        ast->sub_asts_.push_back(new AST(ASTType::AST_EXP));
        exp_exp(ast->sub_asts_.back());
    }
}

void AST::pass_args(AST* ast){
    for(;;){
        if(peek() == TokenType::RIGHT_PARENTHESIS){
            break;
        }

        ast->sub_asts_.push_back(new AST(ASTType ::AST_EXP));
        ast->exp(ast->sub_asts_.back());

        if(peek() == TokenType::RIGHT_PARENTHESIS){
            break;
        }
        next(TokenType::COMMA);
    }
}

void AST::args(AST* ast){
    for(int i = 1;; i ++){
        if(peek() == TokenType::RIGHT_PARENTHESIS){
            break;
        }
        auto token = next(TokenType::SYMBOL);
        ast->sub_asts_.push_back(new AST(ASTType::AST_SYM));
        ast->sub_asts_.back()->ast_value_.str = token->values_;
        ASSERT_EXIT(keywords_token.count(token->values_) == 0, "unexpected keyword(%s) in function args(i)\n", token->values_.c_str(), i);
        if(peek() == TokenType::RIGHT_PARENTHESIS){
            break;
        }
        next(TokenType::COMMA);
    }
}

// --------- stats ------------
void AST::stat_if(AST *ast) {
    ast->ast_type_ = ASTType ::AST_IF;
    next(TokenType::LEFT_PARENTHESIS);
    ast->sub_asts_.push_back(new AST(ASTType ::AST_EXP));
    ast->exp(ast->sub_asts_.back());
    next(TokenType::RIGHT_PARENTHESIS);
    ast->sub_asts_.push_back(new AST(ASTType ::AST_BLOCK));
    ast->stat(ast->sub_asts_.back());
    //  if
    //exp block
}
void AST::stat_for(AST *ast) {
    //          for
    // init_exp, exp, iter_exp, block.

    // expressions: should contains operand and operator(missing)
    ast->ast_type_ = ASTType ::AST_FOR;
    next(TokenType::LEFT_PARENTHESIS);
    ast->sub_asts_.push_back(new AST(ASTType ::AST_EXP));
    ast->exp(ast->sub_asts_.back());
    next(TokenType::SEMICOLON);
    ast->sub_asts_.push_back(new AST(ASTType ::AST_EXP));
    ast->exp(ast->sub_asts_.back());
    next(TokenType::SEMICOLON);
    ast->sub_asts_.push_back(new AST(ASTType ::AST_EXP));
    ast->exp(ast->sub_asts_.back());
    next(TokenType::RIGHT_PARENTHESIS);
    ast->sub_asts_.push_back(new AST(ASTType ::AST_BLOCK));
    ast->stat(ast->sub_asts_.back());
}
void AST::stat_function(AST *ast) {
    //    func
    // args block
    ast->ast_type_ = ASTType ::AST_FUN;
    ast->ast_value_.str = next(TokenType::SYMBOL)->values_;

    next(TokenType::LEFT_PARENTHESIS);
    ast->sub_asts_.push_back(new AST(ASTType ::AST_ARGS));
    ast->args(ast->sub_asts_.back());
    next(TokenType::RIGHT_PARENTHESIS);

    // next(TokenType::LEFT_BRACE);
    ast->sub_asts_.push_back(new AST(ASTType ::AST_BLOCK));
    ast->stat(ast->sub_asts_.back());
    // next(TokenType::RIGHT_BRACE);
}
void AST::stat_exp(AST *ast) {
    //    assign
    // sym      exp
    if(peek() == TokenType::LEFT_PARENTHESIS){
        ast->ast_type_ = ASTType ::AST_CALL;
        ast->ast_value_.str = last_value();
        next(TokenType::LEFT_PARENTHESIS);
        pass_args(ast);
        next(TokenType::RIGHT_PARENTHESIS);
    }else{
        ast->sub_asts_.push_back(new AST(ASTType ::AST_SYM));
        ast->sub_asts_.back()->ast_value_.str = last_value();
        next(TokenType::OP_ASSIGN);
        ast->ast_type_ = ASTType ::AST_ASSIGN;
        ast->sub_asts_.push_back(new AST(ASTType ::AST_EXP));
        ast->exp(ast->sub_asts_.back());
    }
    next(TokenType::SEMICOLON);
}
void AST::stat_return(AST *ast) {
    //   return
    // exp
    ast->ast_type_ = ASTType ::AST_RETURN;
    if(peek() != TokenType::SEMICOLON) {
        ast->sub_asts_.push_back(new AST(ASTType ::AST_EXP));
        ast->exp(ast->sub_asts_.back());
    }
    next(TokenType::SEMICOLON);
}

void AST::stat_brace(AST *ast) {
    ast->ast_type_ = ASTType ::AST_BLOCK;
    while (peek() != TokenType::RIGHT_BRACE) {
        ast->sub_asts_.push_back(new AST());
        ast->stat(ast->sub_asts_.back());
    }
    next(TokenType::RIGHT_BRACE);
}
void AST::stat_print(AST *ast) {
    next(TokenType::LEFT_PARENTHESIS);
    ast->ast_type_ = ASTType ::AST_SYM;
    ast->sub_asts_.push_back(new AST(ASTType ::AST_EXP));
    ast->exp(ast->sub_asts_.back());
    next(TokenType::RIGHT_PARENTHESIS);
    next(TokenType::SEMICOLON);
}
void AST::stat_input(AST *ast) {
    next(TokenType::LEFT_PARENTHESIS);
    ast->ast_type_ = ASTType ::AST_SYM;
    ast->sub_asts_.push_back(new AST(ASTType ::AST_SYM));
    ast->sub_asts_.back()->ast_value_.str = next(TokenType::SYMBOL)->values_;
    next(TokenType::RIGHT_PARENTHESIS);
    next(TokenType::SEMICOLON);
}


void AST::stat(AST* ast){
    auto* current = tokens_[consumed_index_ ++];
    static std::map<TokenType , std::function<void(AST*)>> cb = {
            {TokenType::KEY_IF, std::bind(&AST::stat_if, ast, std::placeholders::_1)},
            {TokenType::KEY_FOR, std::bind(&AST::stat_for, ast, std::placeholders::_1)},
            {TokenType::KEY_FUNCTION, std::bind(&AST::stat_function, ast, std::placeholders::_1)},
            {TokenType::KEY_RETURN, std::bind(&AST::stat_return, ast, std::placeholders::_1)},
            {TokenType::LEFT_BRACE, std::bind(&AST::stat_brace, ast, std::placeholders::_1)},
            {TokenType::SYMBOL, std::bind(&AST::stat_exp, ast, std::placeholders::_1)},
            {TokenType::BUILTIN_PRINT, std::bind(&AST::stat_print, ast, std::placeholders::_1)},
            {TokenType::BUILTIN_INPUT, std::bind(&AST::stat_input, ast, std::placeholders::_1)},
    };

    ASSERT_EXIT(cb.count((TokenType)current->token_type_), "unexpected token: %d", static_cast<int>(current->token_type_));
    cb[(TokenType)current->token_type_](ast);
}

AST* AST::build(std::vector<Token *> &tokens) {
    ASSERT_EXIT(tokens.size() != 0, "no taken can be consumed.");
    consumed_index_ = 0;
    tokens_ = tokens;
    // add builtin functions into env.
    ast_type_ = ASTType ::AST_BLOCK;
    while(consumed_index_ < tokens.size()){
        sub_asts_.push_back(new AST());
        stat(sub_asts_.back());
    }
    return this;
}

void AST::print(AST* ast){
    printf("%s\n", asttype_2_str_[ast->ast_type_].c_str());
    for(size_t i = 0; i < ast->sub_asts_.size(); ++ i){
        printf("    %d,%s\n", i, asttype_2_str_[ast->sub_asts_[i]->ast_type_].c_str());
    }
    printf("\n\n");
}

void AST::print(int deep, AST* ast){
    for(int i = 0; i < deep; ++ i) printf("\t");
    printf("%s", asttype_2_str_[ast->ast_type_].c_str());
    switch (ast->ast_type_){
        case ASTType ::AST_INTEGER: {
            printf("(%d)\n", ast->ast_value_.num);
            break;
        }
        case ASTType ::AST_DECIMAL: {
            printf("(%.2f)\n", ast->ast_value_.dec);
            break;
        }
        case ASTType ::AST_FUN:
        case ASTType ::AST_CALL:
        case ASTType ::AST_SYM: {
            printf("(%s)\n", ast->ast_value_.str.c_str());
            break;
        }
        default:{
            printf("\n");
        }
    }
    for(size_t i = 0; i < ast->sub_asts_.size(); ++ i) print(deep+1, ast->sub_asts_[i]);
}

// ------------------------------ interpret  ------------------------------

void AST::eval_function(AST* ast){
    ASSERT_EXIT(env_.funcs_.count(ast->ast_value_.str) == 0, "function(%s) redefined", ast->ast_value_.str.c_str());
    ASSERT_EXIT(ast->sub_asts_.size() == 2, "function(%s) sub nodes's size(%d)", ast->ast_value_.str.c_str(), ast->sub_asts_.size());
    auto& proto = env_.funcs_[ast->ast_value_.str];
    for(size_t i = 0; i < ast->sub_asts_[0]->sub_asts_.size(); i ++){
        ASSERT_EXIT(ast->sub_asts_[0]->sub_asts_[i]->ast_type_ == ASTType::AST_SYM, "arg(%d) not a symbol (type:%s)",
                i, asttype_2_str_[ast->sub_asts_[0]->sub_asts_[i]->ast_type_].c_str());
        proto.args_symbol_.push_back(ast->sub_asts_[0]->sub_asts_[i]->ast_value_.str);
    }
    proto.ast = ast->sub_asts_[1];
    return ;
}

int AST::eval_builtin(AST* ast){

    auto* env = &env_.global_;
    if(env_.current_.size()){
        env = &env_.current_.back();
    }

    auto fname = ast->ast_value_.str;
    if(fname == "input"){ // TODO: check num/dec/string/char, treat input as num for instance.
        for(int i = 0; i < ast->sub_asts_.size(); ++ i) {
            int v; scanf("%d");
            (*env)[ast->sub_asts_[i]->ast_value_.str].num = v;
            (*env)[ast->sub_asts_[i]->ast_value_.str].value_type_ = (int)ASTType ::AST_INTEGER;
        }
        return 1;
    }
    if(fname == "print"){
        auto print_sym = [](std::string name, const Symbol& symbol){
            if(symbol.value_type_ == (int)ASTType::AST_INTEGER){
                printf("%d", symbol.num);
            }
            else if(symbol.value_type_ == (int)ASTType::AST_DECIMAL){
                printf("%.2f", symbol.dec);
            }else{
                ASSERT_EXIT(false,"symbol can't be print(sym:%s)(type=%d)(num=%d)(dec=%.2f)", name.c_str(), symbol.value_type_, symbol.num, symbol.dec);
            }
        };
        for(int i = 0; i < ast->sub_asts_.size(); ++ i) {
            print_sym("QAQ ", eval_exp(ast->sub_asts_[i]));
            printf("%c", i + 1 == ast->sub_asts_.size() ? '\n' : ' ');
        }
        return 1;
    }
    return 0;
}

Symbol AST::eval_symbol(AST* ast){
    if(env_.current_.size()){
        auto& hsh = env_.current_.back();
        if(hsh.count(ast->ast_value_.str)){
            return hsh[ast->ast_value_.str];
        }
    }
    if(env_.global_.count(ast->ast_value_.str)){
        return env_.global_[ast->ast_value_.str];
    }
    ASSERT_EXIT(false, "use undefined symbol(%s)", ast->ast_value_.str.c_str());
}

Symbol AST::eval_exp(AST* ast){

    auto conv = [](Symbol& sym){
        if(sym.value_type_ != (int)ASTType::AST_DECIMAL){
            sym.value_type_ = (int)ASTType ::AST_DECIMAL;
            sym.dec = sym.num;
        }
    };

    // TODO: exp;
    Symbol result;
    if(ast->ast_type_ == ASTType::AST_EXP){
        return eval_exp(ast->sub_asts_[0]);
    }
    else if(ast->ast_type_ == ASTType::AST_INTEGER){
        result.num = ast->ast_value_.num;
        result.value_type_ = (int)ASTType::AST_INTEGER;
    }
    else if(ast->ast_type_ == ASTType::AST_DECIMAL){
        result.dec = ast->ast_value_.dec;
        result.value_type_ = (int)ASTType::AST_DECIMAL;
    }
    else if(ast->ast_type_ == ASTType::AST_SYM){
        result = eval_symbol(ast);
    }
    else if(ast->ast_type_ == ASTType::AST_ADD){
        // TODO: more type. now only integer & decimal supported.
        Symbol left_operand = eval_exp(ast->sub_asts_[0]);
        Symbol right_operand = eval_exp(ast->sub_asts_[1]);
        if(left_operand.value_type_ != right_operand.value_type_){
            conv(left_operand); conv(right_operand);
            result.value_type_ = (int)ASTType::AST_DECIMAL;
            result.dec = left_operand.dec + right_operand.dec;
        }else {
            result.value_type_ = left_operand.value_type_;
            result.dec = left_operand.dec + right_operand.dec;
            result.num = left_operand.num + right_operand.num;
        }
    }
    else if(ast->ast_type_ == ASTType::AST_SUB){
        // TODO: more type. now only integer & decimal supported.
        Symbol left_operand = eval_exp(ast->sub_asts_[0]);
        Symbol right_operand = eval_exp(ast->sub_asts_[1]);
        if(left_operand.value_type_ != right_operand.value_type_){
            conv(left_operand); conv(right_operand);
            result.value_type_ = (int)ASTType::AST_DECIMAL;
            result.dec = left_operand.dec - right_operand.dec;
        }else {
            result.value_type_ = left_operand.value_type_;
            result.dec = left_operand.dec - right_operand.dec;
            result.num = left_operand.num - right_operand.num;
        }
    }
    else if(ast->ast_type_ == ASTType::AST_CALL){
        result = eval_call(ast);
    }
    else if(ast->ast_type_ == ASTType::AST_LESS_EQUAL){
        Symbol left_operand = eval_exp(ast->sub_asts_[0]);
        Symbol right_operand = eval_exp(ast->sub_asts_[1]);
        if(left_operand.value_type_ != right_operand.value_type_){
            conv(left_operand); conv(right_operand);
            result.value_type_ = (int)ASTType::AST_DECIMAL;
            result.dec = left_operand.dec + right_operand.dec;
        }
        result.value_type_ = (int)ASTType ::AST_INTEGER;
        if(left_operand.value_type_ == (int)ASTType::AST_INTEGER){
            result.num = (left_operand.num <= right_operand.num);
        }else{
            result.num = (left_operand.dec <= right_operand.dec);
        }
    }else{
        ASSERT_EXIT(false, "unexpected asttype(%s)", asttype_2_str_[ast->ast_type_].c_str());
    }
    return result;
}

Symbol AST::eval_assign(AST* ast){
    auto symbol = ast->sub_asts_[0]->ast_value_.str;
    auto val = eval_exp(ast->sub_asts_[1]);
    if(env_.current_.size()){
        auto& hsh = env_.current_.back();
        hsh[symbol] = val;
        return val;
    }
    env_.global_[symbol] = val;
}
Symbol AST::eval_if(AST* ast){
    Symbol symbol;
    symbol.value_type_ = (int)ASTType ::AST_VOID;
    auto val = eval_exp(ast->sub_asts_[0]);
    if(val.value_type_ == (int)ASTType ::AST_INTEGER && val.num){
        auto result = interpret(ast->sub_asts_[1]);
        return result;
    }
    return symbol;
}
Symbol AST::eval_call(AST* ast){
    // TODO: temporary treatment
    if(eval_builtin(ast)){
        return Symbol{};
    }

    std::vector<Symbol> symbols;
    for(int i = 0; i < ast->sub_asts_.size(); ++ i) {
        Symbol tmp = eval_exp(ast->sub_asts_[i]);
        symbols.push_back(tmp);
    }

    env_.current_.emplace_back();
    auto& local = env_.current_.back();
    auto fname = ast->ast_value_.str;
    ASSERT_EXIT(env_.funcs_.count(fname), "use undefined symbol(%s)", fname.c_str());
    auto f = env_.funcs_[fname];
    ASSERT_EXIT(ast->sub_asts_.size() == f.args_symbol_.size(), "use undefined symbol(%s)", fname.c_str());
    for(int i = 0; i < ast->sub_asts_.size(); ++ i) {
        local[f.args_symbol_[i]] = symbols[i];
    }
    auto result_symbol = interpret(f.ast);
    env_.current_.pop_back();
    return result_symbol;
}

Symbol AST::interpret(AST* block) {
    Symbol symbol;
    for(auto* ast : block->sub_asts_){
        if(ast->ast_type_ == ASTType::AST_FUN){
            eval_function(ast);
        }
        if(ast->ast_type_ == ASTType::AST_ASSIGN){
            eval_assign(ast);
        }
        if(ast->ast_type_ == ASTType::AST_BLOCK){
            interpret(ast);
        }
        if(ast->ast_type_ == ASTType::AST_CALL){
            eval_call(ast);// TODO: check return.
        }
        if(ast->ast_type_ == ASTType::AST_IF){
            auto result = eval_if(ast); // TODO: check return.
            if(result.rflag_){
                return result;
            }
        }
        if(ast->ast_type_ == ASTType::AST_RETURN){
            if(!ast->sub_asts_.size()){
                symbol.value_type_ = (int)ASTType ::AST_VOID;
                symbol.rflag_ = 1;
                return symbol;
            }
            auto ret = eval_exp(ast->sub_asts_[0]);
            ret.rflag_ = 1;

            return ret;
        }
    }
    return symbol; // FIXME
    // FIXME: placeholder of print check.
    // FIXME: arg name of function cover global.
    // FIXME: if(){} dele brace in script.
}