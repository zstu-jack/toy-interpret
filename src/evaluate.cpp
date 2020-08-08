#include "evaluate.h"
#include "utils.h"
#include "define.h"
#include "parser.h"
#include <functional>

std::vector<Token *> tokens_;
size_t consumed_index_;
Env env_;

Symbol::Symbol(){
    num = 0;
    dec = 0;
    str = "";
    return_flag_ = 0;

    func.args_symbol_.clear();
    func.ast = nullptr;
    return_flag_ = 0;
}

std::string Symbol::tostring(){
    return "";
}

AST::AST(ASTType type){
    this->ast_type_ = type;
}
AST::AST(){}

TokenType AST::peek_type(){
    if(consumed_index_ == tokens_.size()){
        return TokenType ::END;
    }
    return (TokenType)tokens_[consumed_index_]->token_type_;
}
std::string AST::peek_value(){
    ASSERT_EXIT(consumed_index_ < tokens_.size(), "consumed_index_(%u) exceed", consumed_index_);
    return tokens_[consumed_index_]->values_;
}
std::string AST::last_value(){
    return tokens_[consumed_index_-1]->values_;
}
Token* AST::next(TokenType token_type){
    ASSERT_EXIT(tokens_[consumed_index_]->token_type_ == token_type,"consumed_index=%d, expected 【%s】 but got 【%s】 in line %d\n",
                (int32_t)consumed_index_, tokentype_2_string[token_type].c_str(),
                tokentype_2_string[tokens_[consumed_index_]->token_type_].c_str(),
                tokens_[consumed_index_]->lines_);
    return tokens_[consumed_index_ ++];
}

// ----------------------------------------------------------------------- exp -----------------------------------------------------------------------------------
AST* AST::exp_elem(){
    AST* ast;
    if(peek_type() == TokenType::LEFT_PARENTHESIS){
        next(TokenType::LEFT_PARENTHESIS);
        ast = exp(-1);
        next(TokenType::RIGHT_PARENTHESIS);
    } else if(peek_type() == TokenType::INTEGER){
        ast = new AST(ASTType ::AST_INTEGER);
        ast->ast_value_.num = std::stoi(peek_value());
        next(TokenType::INTEGER);
    } else if(peek_type() == TokenType::DECIMAL){
        ast = new AST(ASTType ::AST_DECIMAL);
        ast->ast_value_.dec = std::stod(peek_value());
        next(TokenType::DECIMAL);
    } else if(peek_type() == TokenType::SYMBOL){
        ast = new AST(ASTType ::AST_SYM);
        ast->ast_value_.str = peek_value();
        next(TokenType::SYMBOL);
        if(peek_type() == TokenType::LEFT_PARENTHESIS){
            ast->ast_type_ = ASTType ::AST_CALL;
            next(TokenType::LEFT_PARENTHESIS);
            pass_args(ast);
            next(TokenType::RIGHT_PARENTHESIS);
        }
    }else if(peek_type() == TokenType::STRING){
        ast = new AST(ASTType ::AST_STRING);
        ast->ast_value_.str = peek_value();
        next(TokenType::STRING);
    }else{
        ASSERT_EXIT(false, "unexpected token type (%s) consume_index = %u\n", tokentype_2_string[peek_type()].c_str(), consumed_index_);
    }
    return ast;
}

AST* AST::exp(int pre){

    // TODO: unary operator
    AST* left = exp_elem();
    if(peek_type() == TokenType::SEMICOLON || peek_type() == TokenType::RIGHT_BRACE || peek_type() == TokenType::COMMA){
        // printf("find[;] at consumed_index = %u\n", consumed_index_);
        return left;
    }
    for(;;){
        TokenType tokenType = peek_type();
        if(tokenType == TokenType::RIGHT_PARENTHESIS || tokenType == TokenType::RIGHT_BRACE || tokenType == TokenType::SEMICOLON || tokenType == TokenType::COMMA){
            break;
        }
        int nxt_precedence = op_precedences[tokenType];
        if(nxt_precedence < pre){
            break;
        }
        next(tokenType);
        AST* right = exp(nxt_precedence+1);
        auto ast_type = tokentype_2_asttype_[tokenType];
        AST* rt = new AST(ast_type);
        rt->sub_asts_.push_back(left);
        rt->sub_asts_.push_back(right);
        left = rt;
    }
    return left;
}

void AST::pass_args(AST* ast){
    for(;;){
        if(peek_type() == TokenType::RIGHT_PARENTHESIS) break;
        ast->sub_asts_.push_back(ast->exp());
        if(peek_type() == TokenType::RIGHT_PARENTHESIS) break;
        next(TokenType::COMMA);
    }
}

void AST::args(AST* ast){
    for(int i = 1;; i ++){
        if(peek_type() == TokenType::RIGHT_PARENTHESIS){
            break;
        }
        auto token = next(TokenType::SYMBOL);
        ast->sub_asts_.push_back(new AST(ASTType::AST_SYM));
        ast->sub_asts_.back()->ast_value_.str = token->values_;
        ASSERT_EXIT(keywords_token.count(token->values_) == 0, "unexpected keyword(%s) in function args(%d)\n", token->values_.c_str(), i);
        if(peek_type() == TokenType::RIGHT_PARENTHESIS){
            break;
        }
        next(TokenType::COMMA);
    }
}

// ----------------------------------------------------------------------- stats -----------------------------------------------------------------------------------
void AST::stat_if(AST *ast) {
    //  if
    //exp block
    ast->ast_type_ = ASTType ::AST_IF;
    next(TokenType::LEFT_PARENTHESIS);
    ast->sub_asts_.push_back(ast->exp());
    next(TokenType::RIGHT_PARENTHESIS);
    ast->sub_asts_.push_back(new AST(ASTType ::AST_BLOCK));
    ast->stat(ast->sub_asts_.back());
}
void AST::stat_while(AST *ast) {
    //        for
    // init?, ?, ?, block.

    //        while
    // exp          block;

    // expressions: should contains operand and operator(missing)
    ast->ast_type_ = ASTType ::AST_WHILE;
    next(TokenType::LEFT_PARENTHESIS);
    ast->sub_asts_.push_back(ast->exp(-1));

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

    ast->sub_asts_.push_back(new AST(ASTType ::AST_BLOCK));
    ast->stat(ast->sub_asts_.back());
}
void AST::stat_exp(AST *ast) {
    //    assign
    // sym      exp
    if(peek_type() == TokenType::LEFT_PARENTHESIS){
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
        ast->sub_asts_.push_back(ast->exp(-1));
    }
    next(TokenType::SEMICOLON);
}
void AST::stat_return(AST *ast) {
    //   return
    // exp
    ast->ast_type_ = ASTType ::AST_RETURN;
    if(peek_type() != TokenType::SEMICOLON) {
        ast->sub_asts_.push_back(ast->exp(-1));
    }
    next(TokenType::SEMICOLON);
}

void AST::stat_brace(AST *ast) {
    ast->ast_type_ = ASTType ::AST_BLOCK;
    while (peek_type() != TokenType::RIGHT_BRACE) {
        ast->sub_asts_.push_back(new AST());
        ast->stat(ast->sub_asts_.back());
    }
    next(TokenType::RIGHT_BRACE);
}
void AST::stat_print(AST *ast) {
    next(TokenType::LEFT_PARENTHESIS);
    ast->ast_type_ = ASTType ::AST_SYM;
    ast->sub_asts_.push_back(ast->exp(-1));
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
            {TokenType::KEY_WHILE, std::bind(&AST::stat_while, ast, std::placeholders::_1)},
            {TokenType::KEY_FUNCTION, std::bind(&AST::stat_function, ast, std::placeholders::_1)},
            {TokenType::KEY_RETURN, std::bind(&AST::stat_return, ast, std::placeholders::_1)},
            {TokenType::LEFT_BRACE, std::bind(&AST::stat_brace, ast, std::placeholders::_1)},
            {TokenType::SYMBOL, std::bind(&AST::stat_exp, ast, std::placeholders::_1)},
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
        printf("    %d,%s\n", (int32_t)i, asttype_2_str_[ast->sub_asts_[i]->ast_type_].c_str());
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

// ----------------------------------------------------------------------- eval expression -----------------------------------------------------------------------------------
Symbol eval_or(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);

    result.value_type_ = left_operand.value_type_;
    switch (left_operand.value_type_){
        case ASTType ::AST_INTEGER: result.num = left_operand.num || right_operand.num; break;
        case ASTType ::AST_STRING:  break;
        case ASTType ::AST_DECIMAL: break;
    }
    return result;
}
Symbol eval_and(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);

    result.value_type_ = left_operand.value_type_;
    switch (left_operand.value_type_){
        case ASTType ::AST_INTEGER: result.num = left_operand.num && right_operand.num; break;
        case ASTType ::AST_STRING:  break;
        case ASTType ::AST_DECIMAL: break;
    }
    return result;
}
Symbol eval_bit_or(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);

    result.value_type_ = left_operand.value_type_;
    switch (left_operand.value_type_){
        case ASTType ::AST_INTEGER: result.num = left_operand.num | right_operand.num; break;
        case ASTType ::AST_STRING:  break;
        case ASTType ::AST_DECIMAL: break;
    }
    return result;
}
Symbol eval_bit_xor(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);

    result.value_type_ = left_operand.value_type_;
    switch (left_operand.value_type_){
        case ASTType ::AST_INTEGER: result.num = left_operand.num ^ right_operand.num; break;
        case ASTType ::AST_STRING:  break;
        case ASTType ::AST_DECIMAL: break;
    }
    return result;
}
Symbol eval_bit_and(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);

    result.value_type_ = left_operand.value_type_;
    switch (left_operand.value_type_){
        case ASTType ::AST_INTEGER: result.num = left_operand.num & right_operand.num; break;
        case ASTType ::AST_STRING:  break;
        case ASTType ::AST_DECIMAL: break;
    }
    return result;
}
Symbol eval_not_equal(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);

    result.value_type_ = ASTType ::AST_INTEGER;
    switch (left_operand.value_type_){
        case ASTType ::AST_INTEGER: result.num = left_operand.num != right_operand.num; break;
        case ASTType ::AST_STRING:  break;
        case ASTType ::AST_DECIMAL: result.num = fabs(left_operand.dec - right_operand.dec) > 1e-5; break;
    }
    return result;
}
Symbol eval_equal(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);

    result.value_type_ = ASTType ::AST_INTEGER;
    switch (left_operand.value_type_){
        case ASTType ::AST_INTEGER: result.num = left_operand.num == right_operand.num; break;
        case ASTType ::AST_STRING:  break;
        case ASTType ::AST_DECIMAL: result.num = fabs(left_operand.dec - right_operand.dec) < 1e-5; break;
    }
    return result;
}
Symbol eval_larger_equal(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);

    result.value_type_ = ASTType ::AST_INTEGER;
    switch (left_operand.value_type_){
        case ASTType ::AST_INTEGER: result.num = left_operand.num >= right_operand.num; break;
        case ASTType ::AST_STRING:  break;
        case ASTType ::AST_DECIMAL: result.num = left_operand.dec >= right_operand.dec; break;
    }
    return result;
}
Symbol eval_larger(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);

    result.value_type_ = ASTType ::AST_INTEGER;
    switch (left_operand.value_type_){
        case ASTType ::AST_INTEGER: result.num = left_operand.num > right_operand.num; break;
        case ASTType ::AST_STRING:  break;
        case ASTType ::AST_DECIMAL: result.num = left_operand.dec > right_operand.dec; break;
    }
    return result;
}
Symbol eval_less_equal(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);
    result.value_type_ = ASTType ::AST_INTEGER;
    switch (left_operand.value_type_) {
        case ASTType::AST_INTEGER: {
            result.num = left_operand.num <= right_operand.num;
            break;
        }
        case ASTType::AST_STRING: {
            result.num = left_operand.str <= right_operand.str;
            break;
        }
        case ASTType::AST_DECIMAL: {
            result.num = left_operand.dec <= right_operand.dec;
            break;
        }
    }
    return result;
}
Symbol eval_less(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);
    result.value_type_ = ASTType ::AST_INTEGER;
    switch (left_operand.value_type_) {
        case ASTType::AST_INTEGER: {
            result.num = left_operand.num < right_operand.num;
            break;
        }
        case ASTType::AST_STRING: {
            result.num = left_operand.str < right_operand.str;
            break;
        }
        case ASTType::AST_DECIMAL: {
            result.num = left_operand.dec < right_operand.dec;
            break;
        }
    }
    return result;
}
Symbol eval_shl(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);

    result.value_type_ = left_operand.value_type_;
    switch (left_operand.value_type_){
        case ASTType ::AST_INTEGER: result.num = left_operand.num << right_operand.num; break;
        case ASTType ::AST_STRING:  break;
        case ASTType ::AST_DECIMAL: break;
    }
    return result;
}
Symbol eval_shr(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);

    result.value_type_ = left_operand.value_type_;
    switch (left_operand.value_type_){
        case ASTType ::AST_INTEGER: result.num = left_operand.num >> right_operand.num; break;
        case ASTType ::AST_STRING:  break;
        case ASTType ::AST_DECIMAL: break;
    }
    return result;
}
Symbol eval_add(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);

    result.value_type_ = left_operand.value_type_;
    switch (left_operand.value_type_){
        case ASTType ::AST_INTEGER: result.num = left_operand.num + right_operand.num; break;
        case ASTType ::AST_STRING:  result.str = left_operand.str + right_operand.str; break;
        case ASTType ::AST_DECIMAL: result.dec = left_operand.dec + right_operand.dec; break;
    }
    return result;
}
Symbol eval_sub(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);
    result.value_type_ = left_operand.value_type_;
    switch (left_operand.value_type_){
        case ASTType ::AST_INTEGER: result.num = left_operand.num - right_operand.num;      break;
        case ASTType ::AST_STRING: ASSERT_EXIT(false, "can't do substract op on string");   break;
        case ASTType ::AST_DECIMAL:result.dec = left_operand.dec - right_operand.dec;       break;
    }
    return result;
}
Symbol eval_mul(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);
    result.value_type_ = left_operand.value_type_;
    switch (left_operand.value_type_){
        case ASTType ::AST_INTEGER: result.num = left_operand.num * right_operand.num;      break;
        case ASTType ::AST_STRING: ASSERT_EXIT(false, "can't do mul op on string");   break;
        case ASTType ::AST_DECIMAL:result.dec = left_operand.dec * right_operand.dec;       break;
    }
    return result;
}
Symbol eval_div(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);
    result.value_type_ = left_operand.value_type_;
    switch (left_operand.value_type_){
        case ASTType ::AST_INTEGER: result.num = left_operand.num / right_operand.num;      break;
        case ASTType ::AST_STRING: ASSERT_EXIT(false, "can't do div op on string");   break;
        case ASTType ::AST_DECIMAL:result.dec = left_operand.dec / right_operand.dec;       break;
    }
    return result;
}
Symbol eval_mod(AST* ast){
    Symbol result;
    Symbol left_operand = ast->eval_exp(ast->sub_asts_[0]);
    Symbol right_operand = ast->eval_exp(ast->sub_asts_[1]);
    result.value_type_ = left_operand.value_type_;
    switch (left_operand.value_type_){
        case ASTType ::AST_INTEGER: result.num = left_operand.num % right_operand.num;      break;
        case ASTType ::AST_STRING: ASSERT_EXIT(false, "can't do mod op on string");   break;
        case ASTType ::AST_DECIMAL:ASSERT_EXIT(false, "can't do mod op on decimal");   break;
    }
    return result;
}
// ------------------------------------------------------------------------------------------ interpret  ------------------------------------------------------------------------------------------

Symbol AST::eval_function(AST* ast){
    ASSERT_EXIT(env_.funcs_.count(ast->ast_value_.str) == 0, "function(%s) redefined", ast->ast_value_.str.c_str());
    ASSERT_EXIT(ast->sub_asts_.size() == 2, "function(%s) sub nodes's size(%d)", ast->ast_value_.str.c_str(), (int32_t)ast->sub_asts_.size());
    auto& proto = env_.funcs_[ast->ast_value_.str];
    for(size_t i = 0; i < ast->sub_asts_[0]->sub_asts_.size(); i ++){
        ASSERT_EXIT(ast->sub_asts_[0]->sub_asts_[i]->ast_type_ == ASTType::AST_SYM, "arg(%d) not a symbol (type:%s)",
                    (int32_t)i, asttype_2_str_[ast->sub_asts_[0]->sub_asts_[i]->ast_type_].c_str());
        proto.args_symbol_.push_back(ast->sub_asts_[0]->sub_asts_[i]->ast_value_.str);
    }
    proto.ast = ast->sub_asts_[1];
    return {};
}

int AST::is_builtin(AST *ast) {
    Symbol symbol{};
    symbol.num = 1;

    auto* env = &env_.global_;
    if(env_.current_.size()){
        env = &env_.current_.back();
    }

    auto fname = ast->ast_value_.str;

    return fname == "input" || fname == "print";
}

Symbol AST::eval_builtin(AST* ast){

    Symbol symbol{};
    symbol.num = 1;

    auto* env = &env_.global_;
    if(env_.current_.size()){
        env = &env_.current_.back();
    }

    auto fname = ast->ast_value_.str;
    if(fname == "input"){ // TODO: support more data type and more number of args for input.
        //for(size_t i = 0; i < ast->sub_asts_.size(); ++ i) {
        int v; scanf("%d", &v);
        Symbol result;
        result.num = v;
        result.value_type_ = ASTType ::AST_INTEGER;
        //}
        return result;
    }
    if(fname == "print"){
        auto print_sym = [=](std::string name, const Symbol& symbol){
            if(symbol.value_type_ == ASTType::AST_INTEGER){
                printf("%d", symbol.num);
            }
            else if(symbol.value_type_ == ASTType::AST_DECIMAL){
                printf("%.2f", symbol.dec);
            }else if(symbol.value_type_ == ASTType::AST_STRING){
                printf("%s", symbol.str.c_str());
            }else{
                print(ast);
                ASSERT_EXIT(false,"symbol can't be print(sym:%s)(type=%d)(num=%d)(dec=%.2f)", name.c_str(), symbol.value_type_, symbol.num, symbol.dec);
            }
        };
        for(size_t i = 0; i < ast->sub_asts_.size(); ++ i) {
            print_sym("[QAQ]", eval_exp(ast->sub_asts_[i]));
            printf("%c", i + 1 == ast->sub_asts_.size() ? '\n' : ' ');
        }
        return symbol;
    }
    symbol.num = 0;
    return symbol;
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

    // TODO: wrapper exp and support more operate and conversion of operands
    Symbol result;
    if(ast->ast_type_ == ASTType::AST_EXP){
        return eval_exp(ast->sub_asts_[0]);
    }
    else if(ast->ast_type_ == ASTType::AST_STRING){
        result.str = ast->ast_value_.str;
        result.value_type_ = ASTType::AST_STRING;
    }
    else if(ast->ast_type_ == ASTType::AST_INTEGER){
        result.num = ast->ast_value_.num;
        result.value_type_ = ASTType::AST_INTEGER;
    }
    else if(ast->ast_type_ == ASTType::AST_DECIMAL){
        result.dec = ast->ast_value_.dec;
        result.value_type_ = ASTType::AST_DECIMAL;
    }
    else if(ast->ast_type_ == ASTType::AST_SYM){
        result = eval_symbol(ast);
    }
    else if(ast->ast_type_ == ASTType::AST_CALL){
        result = eval_call(ast);
    }
    else if(arith_callback_.count(ast->ast_type_)){
        result = arith_callback_[ast->ast_type_](ast);
    }else{
        print(ast);
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
    return env_.global_[symbol] = val;
}
Symbol AST::eval_while(AST* ast){
    Symbol symbol;
    symbol.value_type_ = ASTType ::AST_VOID;
    auto val = eval_exp(ast->sub_asts_[0]);
    while (val.value_type_ == ASTType ::AST_INTEGER && val.num){
        auto result = interpret(ast->sub_asts_[1]);
        if(result.return_flag_){
            return result;
        }
        val = eval_exp(ast->sub_asts_[0]);
    }
    return symbol;
}
Symbol AST::eval_if(AST* ast){
    Symbol symbol;
    symbol.value_type_ = ASTType ::AST_VOID;
    auto val = eval_exp(ast->sub_asts_[0]);
    if(val.value_type_ == ASTType ::AST_INTEGER && val.num){
        auto result = interpret(ast->sub_asts_[1]);
        return result;
    }
    return symbol;
}
Symbol AST::eval_call(AST* ast){

    if(is_builtin(ast)){
        return eval_builtin(ast);
    }

    std::vector<Symbol> symbols;
    for(size_t i = 0; i < ast->sub_asts_.size(); ++ i) {
        Symbol tmp = eval_exp(ast->sub_asts_[i]);
        symbols.push_back(tmp);
    }

    env_.current_.emplace_back();
    auto& local = env_.current_.back();
    auto fname = ast->ast_value_.str;
    ASSERT_EXIT(env_.funcs_.count(fname), "use undefined symbol(%s)", fname.c_str());
    auto f = env_.funcs_[fname];
    ASSERT_EXIT(ast->sub_asts_.size() == f.args_symbol_.size(), "use undefined symbol(%s)", fname.c_str());
    for(size_t i = 0; i < ast->sub_asts_.size(); ++ i) {
        local[f.args_symbol_[i]] = symbols[i];
    }
    auto result_symbol = interpret(f.ast);
    env_.current_.pop_back();
    return result_symbol;
}

Symbol AST::interpret(AST* block) {
    Symbol symbol{};
    for(auto* ast : block->sub_asts_){
        if(ast->ast_type_ == ASTType::AST_FUN)    eval_function(ast);
        if(ast->ast_type_ == ASTType::AST_ASSIGN) eval_assign(ast);
        if(ast->ast_type_ == ASTType::AST_BLOCK)  symbol = interpret(ast);
        if(ast->ast_type_ == ASTType::AST_CALL)   eval_call(ast);
        if(ast->ast_type_ == ASTType::AST_IF)     symbol = eval_if(ast);
        if(ast->ast_type_ == ASTType::AST_WHILE)  symbol = eval_while(ast);
        if(ast->ast_type_ == ASTType::AST_RETURN){
            if(!ast->sub_asts_.size()){
                symbol.value_type_ = ASTType ::AST_VOID;
                symbol.return_flag_ = 1;
                return symbol;
            }
            auto ret = eval_exp(ast->sub_asts_[0]);
            ret.return_flag_ = 1;
            return ret;
        }
        if(symbol.return_flag_) return symbol;
    }
    return symbol;
    // FIXME: placeholder of print check.
    // FIXME: if(){} dele brace in script.
    // TODO: enter block like(if/while) cover the local variable;
}


std::map<ASTType , std::function<Symbol(AST*)>> arith_callback_ = {
    {ASTType ::AST_OR, std::bind(eval_or, std::placeholders::_1)},
    {ASTType ::AST_AND,std::bind(eval_and, std::placeholders::_1)},
    {ASTType ::AST_BIT_OR,std::bind(eval_bit_or, std::placeholders::_1)},
    {ASTType ::AST_BIT_XOR,std::bind(eval_bit_xor, std::placeholders::_1)},
    {ASTType ::AST_BIT_AND,std::bind(eval_and, std::placeholders::_1)},
    {ASTType ::AST_NOT_EQUAL,std::bind(eval_not_equal, std::placeholders::_1)},
    {ASTType ::AST_EQUAL,std::bind(eval_equal, std::placeholders::_1)},
    {ASTType ::AST_LARGER_EQUAL,std::bind(eval_larger_equal, std::placeholders::_1)},
    {ASTType ::AST_LARGER,std::bind(eval_larger, std::placeholders::_1)},
    {ASTType ::AST_LESS_EQUAL,std::bind(eval_less_equal, std::placeholders::_1)},
    {ASTType ::AST_LESS,std::bind(eval_less, std::placeholders::_1)},
    {ASTType ::AST_SHL,std::bind(eval_shl, std::placeholders::_1)},
    {ASTType ::AST_SHR,std::bind(eval_shr, std::placeholders::_1)},
    {ASTType ::AST_ADD,std::bind(eval_add, std::placeholders::_1)},
    {ASTType ::AST_SUB,std::bind(eval_sub, std::placeholders::_1)},
    {ASTType ::AST_MUL,std::bind(eval_mul, std::placeholders::_1)},
    {ASTType ::AST_DIV,std::bind(eval_div, std::placeholders::_1)},
    {ASTType ::AST_MOD,std::bind(eval_mod, std::placeholders::_1)}
};