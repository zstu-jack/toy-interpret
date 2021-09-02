#include "evaluate.h"
#include "utils.h"
#include "define.h"
#include "parser.h"
#include <functional>
#include <cmath>
#include <stdlib.h>
#include <iostream>

#if defined(__linux__)
#include <sys/time.h>
#include <utime.h>
#include <sys/types.h>
#include <unistd.h>
#else
#endif

Tokenizer AST::tokenizer_;
std::vector<Token> AST::tokens_;
int32_t AST::token_index_;
Env AST::env_;

Symbol::Symbol(){
    num = 0;
    dec = 0;
    str = "";
    return_flag_ = 0;
    object = nullptr;

    func.args_symbol_.clear();
    func.ast = nullptr;
}
Symbol::~Symbol(){
    if(type_ == ASTType::AST_OBJECT){
        if(object == nullptr){
            return ;
        }
        delete object;
        object = nullptr;
    }
}
void Symbol::FieldCopy(const Symbol& symbol) {
    this->type_ = symbol.type_;
    this->str = symbol.str;
    this->num = symbol.num;
    this->dec = symbol.dec;
    this->func = symbol.func;
    this->return_flag_ = symbol.return_flag_;
}
Symbol::Symbol(const Symbol& symbol){
    this->object = nullptr;
    FieldCopy(symbol);
    if(symbol.object != nullptr){
        this->object = new Object;
        (*(this->object)) = *symbol.object;
    }
}
Symbol& Symbol::operator=(const Symbol& symbol){
    this->object = nullptr;
    FieldCopy(symbol);
    if(symbol.object != nullptr){
        this->object = new Object;
        (*(this->object)) = *symbol.object;
    }
    return *this;
}
Symbol::Symbol(Symbol&& symbol){
    FieldCopy(symbol);
    this->object = symbol.object;
    symbol.object = nullptr;
}
Symbol& Symbol::operator=(Symbol&& symbol){
    FieldCopy(symbol);
    this->object = symbol.object;
    symbol.object = nullptr;
    return *this;
}

std::string Symbol::tostring(){
    return asttype_2_string[this->type_] + " | " + std::to_string(num) + " | " + std::to_string(dec) + " | " + str;
}

AST::AST(ASTType type){
    this->ast_type_ = type;
}
AST::AST(ASTType type, const std::string& str){
    this->ast_type_ = type;
    this->ast_value_.str = str;
}
AST::AST(){}

TokenType AST::peek_type(){
    if(token_index_ == tokens_.size()){
        return TokenType ::END;
    }
    return (TokenType)tokens_[token_index_].token_type_;
}
std::string AST::peek_value(){
    ASSERT_EXIT(token_index_ < tokens_.size(), "token_index_(%lu) exceed", token_index_);
    return tokens_[token_index_].values_;
}
std::string AST::last_value(){
    return tokens_[token_index_-1].values_;
}
void AST::check_next(TokenType token_type){
    ASSERT_EXIT(tokens_[token_index_].token_type_ == token_type,"\nline %d: %s\nline %d: %s\n     expected 【%s】 but got 【%s】in line %d \n\n",
                tokens_[token_index_].lines_ - 1,
                tokenizer_._line2line_str[tokens_[token_index_].lines_ - 1].c_str(),
                tokens_[token_index_].lines_,
                tokenizer_._line2line_str[tokens_[token_index_].lines_].c_str(),
                tokentype_2_string[token_type].c_str(),
                tokentype_2_string[tokens_[token_index_].token_type_].c_str(),
                tokens_[token_index_].lines_);
}
Token AST::next(TokenType token_type){
    check_next(token_type);
    return tokens_[token_index_ ++];
}



// ----------------------------------------------------------------------- exp -----------------------------------------------------------------------------------
AST* AST::stat_exp_elem(){
    AST* ast;

    if(peek_type() == TokenType::LEFT_BRACE){
        /** {arg, arg, arg, ...} */
        next(TokenType::LEFT_BRACE);
        ast = new AST(ASTType::AST_INITLIST);
        stat_parameters_passed(ast, TokenType::RIGHT_BRACE);
        next(TokenType::RIGHT_BRACE);
    } else if(peek_type() == TokenType::LEFT_PARENTHESIS){
        /** (...) */
        next(TokenType::LEFT_PARENTHESIS);
        ast = stat_exp(-1);
        next(TokenType::RIGHT_PARENTHESIS);
    } else if(peek_type() == TokenType::INTEGER){
        /** integer */
        ast = new AST(ASTType ::AST_INTEGER);
        ast->ast_value_.num = std::stoi(peek_value());
        next(TokenType::INTEGER);
    } else if(peek_type() == TokenType::DECIMAL){
        /** decimal */
        ast = new AST(ASTType ::AST_DECIMAL);
        ast->ast_value_.dec = std::stod(peek_value());
        next(TokenType::DECIMAL);
    }else if(peek_type() == TokenType::STRING){
        /** str */
        ast = new AST(ASTType ::AST_STRING);
        ast->ast_value_.str = peek_value();
        next(TokenType::STRING);
    }else if( peek_type() == TokenType::SYMBOL){
        /**
         * symbol
         *      symbol[index].
         *      symbol.xxx.
         *      symbol()
         * */
        /* symbol */
        ast = new AST(ASTType ::AST_SYM);
        ast->ast_value_.str = peek_value();
        next(TokenType::SYMBOL);

        auto sym = ast;
        auto type = peek_type();
        while (type == TokenType::DOT || type == TokenType::LEFT_BRACKET){
            switch(type){
                case TokenType::LEFT_BRACKET:
                    next(TokenType::LEFT_BRACKET);
                    sym->sub_asts_.push_back(new AST(ASTType::AST_INTEGER));
                    sym->sub_asts_.back()->sub_asts_.push_back(ast ->stat_exp(-1));
                    next(TokenType::RIGHT_BRACKET);
                    break;
                case TokenType::DOT:
                    next(TokenType::DOT);
                    sym->sub_asts_.push_back(new AST(ASTType::AST_STRING));
                    sym->sub_asts_.back()->sub_asts_.push_back(ast ->stat_exp(-1));
                    break;
            }
            type = peek_type();
        }
        if(peek_type() == TokenType::LEFT_PARENTHESIS){
            ast->ast_type_ = ASTType ::AST_CALL;
            next(TokenType::LEFT_PARENTHESIS);
            stat_parameters_passed(ast, TokenType::RIGHT_PARENTHESIS);
            next(TokenType::RIGHT_PARENTHESIS);
        }
    }else {
        ASSERT_EXIT(false, "unexpected token type (`%s`) token_index = %lu\n", tokentype_2_string[peek_type()].c_str(), token_index_);
    }
    return ast;
}

AST* AST::stat_exp(int pre){

    // TODO: unary operator
    AST* left = stat_exp_elem();

    if(peek_type() == TokenType::SEMICOLON          // ;
    || peek_type() == TokenType::COMMA              // ,
    || peek_type() == TokenType::RIGHT_BRACE        // }
    || peek_type() == TokenType::RIGHT_BRACKET      // ]
    ){
        return left;
    }


    for(;;){
        TokenType tokenType = peek_type();
        if(tokenType == TokenType::RIGHT_PARENTHESIS    // )
        || tokenType == TokenType::RIGHT_BRACE          // }
        || tokenType == TokenType::RIGHT_BRACKET        // ]
        || tokenType == TokenType::SEMICOLON            // ;
        || tokenType == TokenType::COMMA){              // ,
            break;
        }
        int nxt_precedence = operator_precedences[tokenType];
        if(nxt_precedence < pre){
            break;
        }
        next(tokenType);
        AST* right = stat_exp(nxt_precedence+1);
        auto ast_type = tokentype_2_asttype_[tokenType];
        AST* rt = new AST(ast_type);
        rt->sub_asts_.push_back(left);
        rt->sub_asts_.push_back(right);
        left = rt;
    }
    return left;
}

void AST::stat_parameters_passed(AST* ast, TokenType end){
    for(;;){
        if(peek_type() == end) break;
        ast->sub_asts_.push_back(ast->stat_exp());
        if(peek_type() == end) break;
        next(TokenType::COMMA);
    }
}

AST* AST::stat_function_args(){
    //    function_args
    // symbol symbol symbol ...

    AST* ast = new AST(ASTType::AST_ARGS);
    for(int i = 1;; i ++){
        if(peek_type() == TokenType::RIGHT_PARENTHESIS){
            break;
        }
        auto token = next(TokenType::SYMBOL);
        ast->sub_asts_.push_back(new AST(ASTType::AST_SYM));
        ast->sub_asts_.back()->ast_value_.str = token.values_;
        ASSERT_EXIT(keywords_2_token_type.count(token.values_) == 0, "unexpected keyword(%s) in function args(%d)\n", token.values_.c_str(), i);
        if(peek_type() == TokenType::RIGHT_PARENTHESIS){
            break;
        }
        next(TokenType::COMMA);
    }
    return ast;
}

// ----------------------------------------------------------------------- stats -----------------------------------------------------------------------------------
AST* AST::stat_if() {
    //   if
    //exp  {block}
    AST* ast = new AST(ASTType ::AST_IF);
    next(TokenType::KEY_IF);
    next(TokenType::LEFT_PARENTHESIS);
    ast->sub_asts_.push_back(ast ->stat_exp());
    next(TokenType::RIGHT_PARENTHESIS);

    check_next(TokenType::LEFT_BRACE);
    ast->sub_asts_.push_back(stat());
    return ast;
}
AST* AST::stat_while() {
    //        while
    // exp          block;


    AST* ast = new AST(ASTType ::AST_WHILE);
    next(TokenType::KEY_WHILE);
    next(TokenType::LEFT_PARENTHESIS);
    ast->sub_asts_.push_back(ast ->stat_exp(-1));
    next(TokenType::RIGHT_PARENTHESIS);

    check_next(TokenType::LEFT_BRACE);
    ast->sub_asts_.push_back(stat());
    return ast;
}
AST* AST::stat_function() {
    //    func
    // args   block
    AST* ast = new AST(ASTType ::AST_FUN);
    next(TokenType::KEY_FUNCTION);
    ast->ast_value_.str = next(TokenType::SYMBOL).values_;

    next(TokenType::LEFT_PARENTHESIS);
    ast->sub_asts_.push_back(ast->stat_function_args());

    next(TokenType::RIGHT_PARENTHESIS);
    ast->sub_asts_.push_back(ast->stat());
    return ast;
}
AST* AST::stat_exp_semicolon() {
    // etc: a = 3;  a = b * c;
    AST* ast = stat_exp(-1);
    next(TokenType::SEMICOLON);
    return ast;
}
AST* AST::stat_return() {
    //   return
    // exp;
    AST* ast = new AST(ASTType ::AST_RETURN);
    next(TokenType::KEY_RETURN);
    if(peek_type() != TokenType::SEMICOLON) {
        ast->sub_asts_.push_back(ast ->stat_exp(-1));
    }
    next(TokenType::SEMICOLON);
    return ast;
}

AST* AST::stat_block() {
    //      block
    // stat stat stat .....

    AST* ast = new AST(ASTType ::AST_BLOCK);
    next(TokenType::LEFT_BRACE);
    while (peek_type() != TokenType::RIGHT_BRACE) {
        ast->sub_asts_.push_back(ast->stat());
    }
    next(TokenType::RIGHT_BRACE);
    return ast;
}


AST* AST::stat(){
    auto current = tokens_[token_index_];
    static std::map<TokenType , std::function<AST*()> > cb = {
            {TokenType::KEY_IF, std::bind(&AST::stat_if, this)},
            {TokenType::KEY_WHILE, std::bind(&AST::stat_while, this)},
            {TokenType::KEY_FUNCTION, std::bind(&AST::stat_function, this)},
            {TokenType::KEY_RETURN, std::bind(&AST::stat_return, this)},
            {TokenType::LEFT_BRACE, std::bind(&AST::stat_block, this)},
            {TokenType::SYMBOL, std::bind(&AST::stat_exp_semicolon, this)},
    };

    ASSERT_EXIT(cb.count((TokenType)current.token_type_), "unexpected token: %d", static_cast<int>(current.token_type_));
    return cb[(TokenType)current.token_type_]();
}

AST* AST::build(Tokenizer tokenizer) {
    tokenizer_ = tokenizer;
    tokens_ = tokenizer._tokens;
    ASSERT_EXIT(tokens_.size() != 0, "no token can be consumed.");

    // add builtin functions into env.
    ast_type_ = ASTType ::AST_BLOCK;

    token_index_ = 0;
    while(token_index_ < tokens_.size()){
        sub_asts_.push_back(stat());
    }
    return this;
}

void AST::print(AST* ast){
    printf("%s\n", asttype_2_string[ast->ast_type_].c_str());
    for(size_t i = 0; i < ast->sub_asts_.size(); ++ i){
        printf("    %d,%s\n", (int32_t)i, asttype_2_string[ast->sub_asts_[i]->ast_type_].c_str());
    }
    printf("\n\n");
}

void AST::print(int deep, AST* ast){
    for(int i = 0; i < deep; ++ i) printf("\t");
    printf("%s", asttype_2_string[ast->ast_type_].c_str());
    switch (ast->ast_type_){
        case ASTType ::AST_INTEGER: {
            printf("(%lld)\n", ast->ast_value_.num);
            break;
        }
        case ASTType ::AST_DECIMAL: {
            printf("(%.2f)\n", ast->ast_value_.dec);
            break;
        }
        case ASTType ::AST_STRING: {
            printf("(%s)\n", ast->ast_value_.str.c_str());
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
Symbol& Symbol::operator+(const Symbol& symbol){
    ASSERT_EXIT(symbol.type_ == type_, "type of left&right operand should be same, lhs type(%s), lhs name(%s), rhs type(%s), rhs name(%s)",
                asttype_2_string[type_].c_str(), str.c_str(), asttype_2_string[symbol.type_].c_str(), symbol.str.c_str());
    switch (type_){
        case ASTType ::AST_INTEGER: num += symbol.num; break;
        case ASTType ::AST_STRING:  str += symbol.str; break;
        case ASTType ::AST_DECIMAL: dec += symbol.dec; break;
        default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
    }
    return *this;
}
Symbol& Symbol::operator-(const Symbol& symbol){
    ASSERT_EXIT(symbol.type_ == type_, "type of left&right operand should be same");
    switch (type_){
        case ASTType ::AST_INTEGER: num -= symbol.num; break;
        case ASTType ::AST_DECIMAL: dec -= symbol.dec; break;
        default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
    }
    return *this;
}
Symbol& Symbol::operator*(const Symbol& symbol){
    ASSERT_EXIT(symbol.type_ == type_, "type of left&right operand should be same");
    switch (type_){
        case ASTType ::AST_INTEGER: num *= symbol.num; break;
        case ASTType ::AST_DECIMAL: dec *= symbol.dec; break;
        default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
    }
    return *this;
}
Symbol& Symbol::operator/(const Symbol& symbol){
    ASSERT_EXIT(symbol.type_ == type_, "type of left&right operand should be same");
    switch (type_){
        case ASTType ::AST_INTEGER: num /= symbol.num; break;
        case ASTType ::AST_DECIMAL: dec /= symbol.dec; break;
        default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
    }
    return *this;
}
Symbol& Symbol::operator%(const Symbol& symbol){
    ASSERT_EXIT(symbol.type_ == type_, "type of left&right operand should be same");
    switch (type_){
        case ASTType ::AST_INTEGER: num %= symbol.num; break;
        default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
    }
    return *this;
}
Symbol& Symbol::operator||(const Symbol& symbol){
    ASSERT_EXIT(symbol.type_ == type_, "type of left&right operand should be same");
    int32_t result = 0;
    switch (type_){
        case ASTType ::AST_INTEGER: result |= num != 0; break;
        case ASTType ::AST_DECIMAL: result |= dec != 0; break;
        default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
    }
    if(!result){
        switch (symbol.type_) {
            case ASTType ::AST_INTEGER: result |= (symbol.num != 0); break;
            case ASTType ::AST_DECIMAL: result |= (symbol.dec != 0); break;
            default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
        }
    }
    type_ = ASTType::AST_INTEGER;
    num = result;
    return (*this);
}
Symbol& Symbol::operator&&(const Symbol& symbol){
    ASSERT_EXIT(symbol.type_ == type_, "type of left&right operand should be same");
    int32_t result = 0;
    switch (type_){
        case ASTType ::AST_INTEGER: result = num != 0; break;
        case ASTType ::AST_DECIMAL: result = dec != 0; break;
        default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
    }
    if(result) {
        switch (symbol.type_) {
            case ASTType::AST_INTEGER: result = symbol.num != 0; break;
            case ASTType::AST_DECIMAL: result = symbol.dec != 0; break;
            default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
        }
    }
    type_ = ASTType::AST_INTEGER;
    num = result;
    return (*this);
}

Symbol& Symbol::operator|(const Symbol& symbol){
    ASSERT_EXIT(symbol.type_ == type_, "type of left&right operand should be same");
    switch (type_){
        case ASTType ::AST_INTEGER: num |= symbol.num; break;
        default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
    }
    return *this;
}
Symbol& Symbol::operator&(const Symbol& symbol){
    ASSERT_EXIT(symbol.type_ == type_, "type of left&right operand should be same");
    switch (type_){
        case ASTType ::AST_INTEGER: num &= symbol.num; break;
        default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
    }
    return *this;
}
Symbol& Symbol::operator^(const Symbol& symbol){
    ASSERT_EXIT(symbol.type_ == type_, "type of left&right operand should be same");
    switch (type_){
        case ASTType ::AST_INTEGER: num ^= symbol.num; break;
        default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
    }
    return *this;
}
Symbol& Symbol::operator!=(const Symbol& symbol){
    ASSERT_EXIT(symbol.type_ == type_, "type of left&right operand should be same");
    int32_t result = 0;
    switch (type_){
        case ASTType ::AST_INTEGER: result = num != symbol.num; break;
        case ASTType ::AST_STRING:  result = str != symbol.str; break;
        case ASTType ::AST_DECIMAL: result = dec != symbol.dec; break;
        default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
    }
    type_ = ASTType ::AST_INTEGER;
    num = result;
    return *this;
}
Symbol& Symbol::operator==(const Symbol& symbol){
    // *this = (*this != symbol);
    this->operator!=(symbol);
    this->num = !this->num;
}
Symbol& Symbol::operator>(const Symbol& symbol){
    ASSERT_EXIT(symbol.type_ == type_, "type of left&right operand should be same");
    int32_t result = 0;
    switch (type_){
        case ASTType ::AST_INTEGER: result = num > symbol.num; break;
        case ASTType ::AST_DECIMAL: result = dec > symbol.dec; break;
        default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
    }
    this->num = result;
    this->type_ = ASTType::AST_INTEGER;
    return *this;
}
Symbol& Symbol::operator>=(const Symbol& symbol){
    ASSERT_EXIT(symbol.type_ == type_, "type of left&right operand should be same");
    int32_t result = 0;
    switch (type_){
        case ASTType ::AST_INTEGER: result = num >= symbol.num; break;
        case ASTType ::AST_DECIMAL: result = dec >= symbol.dec; break;
        default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
    }
    this->num = result;
    this->type_ = ASTType::AST_INTEGER;
    return *this;
}
Symbol& Symbol::operator<(const Symbol& symbol){
    this->operator>=(symbol);
    this->num = !this->num;
    return *this;
}
Symbol& Symbol::operator<=(const Symbol& symbol){
    // *this = (*this > symbol);
    this->operator>(symbol);
    this->num = !this->num;
    return *this;
}
Symbol& Symbol::operator<<(const Symbol& symbol){
    ASSERT_EXIT(symbol.type_ == type_, "type of left&right operand should be same");
    switch (type_){
        case ASTType ::AST_INTEGER: num <<= symbol.num; break;
        default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
    }
    return *this;
}
Symbol& Symbol::operator>>(const Symbol& symbol){
    ASSERT_EXIT(symbol.type_ == type_, "type of left&right operand should be same");
    switch (type_){
        case ASTType ::AST_INTEGER: num >>= symbol.num; break;
        default: ASSERT_EXIT(false, "unsupported type of operator function(%s), type(%s)\n", __func__, asttype_2_string[type_].c_str());
    }
    return *this;
}



// ------------------------------------------------------------------------------------------ interpret  ------------------------------------------------------------------------------------------

Symbol AST::eval_function(AST* ast){
    ASSERT_EXIT(env_.funcs_.count(ast->ast_value_.str) == 0, "function(%s) redefined", ast->ast_value_.str.c_str());
    ASSERT_EXIT(ast->sub_asts_.size() == 2, "function(%s) sub nodes's size(%d)", ast->ast_value_.str.c_str(), (int32_t)ast->sub_asts_.size());
    // args, body
    auto& proto = env_.funcs_[ast->ast_value_.str];
    for(size_t i = 0; i < ast->sub_asts_[0]->sub_asts_.size(); i ++){
        ASSERT_EXIT(ast->sub_asts_[0]->sub_asts_[i]->ast_type_ == ASTType::AST_SYM, "arg(%d) not a symbol (type:%s)",
                    (int32_t)i, asttype_2_string[ast->sub_asts_[0]->sub_asts_[i]->ast_type_].c_str());
        proto.args_symbol_.push_back(ast->sub_asts_[0]->sub_asts_[i]->ast_value_.str);
    }
    proto.ast = ast->sub_asts_[1];
    return {};
}

int AST::is_builtin(AST *ast) {
    auto func_name = ast->ast_value_.str;
    return func_name == "input" || func_name == "print" || func_name == "time";
}

void AST::print_symbol(std::string name, const Symbol& symbol){
    if(symbol.type_ == ASTType::AST_INTEGER){
        printf("%lld", symbol.num);
    }
    else if(symbol.type_ == ASTType::AST_DECIMAL){
        printf("%.2f", symbol.dec);
    }else if(symbol.type_ == ASTType::AST_STRING){
        printf("%s", symbol.str.c_str());
    }else if(symbol.type_ == ASTType::AST_OBJECT){
        ASSERT_EXIT(symbol.object != nullptr, "object == nullptr");
        for(auto& pi : symbol.object->obj_){
            printf("<%d = ", pi.first);
            print_symbol(name, pi.second);
            printf(">  ");
        }
    }
    else{
//                print(ast);
        ASSERT_EXIT(false, "symbol can't be print(sym:%s)(type=%s)(num=%lld)(dec=%.2f)",
                    name.c_str(), asttype_2_string[symbol.type_].c_str(), symbol.num, symbol.dec);
    }
}

Symbol AST::eval_builtin(AST* ast){

    Symbol result;

    auto func_name = ast->ast_value_.str;
    if(func_name == "input"){
        int v; scanf("%d", &v);
        result.num = v;
        result.type_ = ASTType ::AST_INTEGER;
        return result;
    }

    if(func_name == "time"){

        result.type_ = ASTType ::AST_INTEGER;
#if defined(__linux__)
        struct timeval t_start;
        gettimeofday(&t_start, NULL);
        result.num = (t_start.tv_sec*1000000 + t_start.tv_usec);
#else
        result.num = 0; // FIXME: not supported windows for now.
#endif
        return result;
    }

    if(func_name == "print"){
        for(size_t i = 0; i < ast->sub_asts_.size(); ++ i) {
            print_symbol("[QAQ]", eval_exp(ast->sub_asts_[i]));
            printf("%c", i + 1 == ast->sub_asts_.size() ? '\n' : ' ');
        }
        return result;
    }
    ASSERT_EXIT(false, "not built-in functions (%s)\n", func_name.c_str());
}

Symbol AST::eval_symbol(AST* ast){

    if(env_.current_.size()){
        auto& hsh = env_.current_.back();
//        for(auto symbol :hsh){
//            printf("env symbol: %s \n", symbol.first.c_str());
//        }
        if(hsh.count(ast->ast_value_.str)){
            return hsh[ast->ast_value_.str];
        }
    }
//    for(auto symbol :env_.global_){
//        printf("global symbol: %s \n", symbol.first.c_str());
//    }
    if(env_.global_.count(ast->ast_value_.str)){
        return env_.global_[ast->ast_value_.str];
    }
    ASSERT_EXIT(false, "use undefined symbol(%s)\n", ast->ast_value_.str.c_str());
}

Symbol AST::eval_exp(AST* ast){
    Symbol result;
    if(ast->ast_type_ == ASTType::AST_EXP){
        return eval_exp(ast->sub_asts_[0]);
    }
    else if(ast->ast_type_ == ASTType::AST_INITLIST){
        // TODO: abstract object as a symbol.
        //       Now use object as a operand.
        result.type_ = ASTType ::AST_OBJECT;
        result.object = new Object;
        auto& mp = result.object->obj_;
        for(int i = 0; i < ast->sub_asts_.size(); ++ i){
            mp[i] = eval_exp(ast->sub_asts_[i]);
        }
    }
    else if(ast->ast_type_ == ASTType::AST_OBJECT){
        result.object = new Object;
        (*result.object) = (*ast->ast_value_.object);
        result.type_ = ASTType ::AST_OBJECT;
    }
    else if(ast->ast_type_ == ASTType::AST_STRING){
        result.str = ast->ast_value_.str;
        result.type_ = ASTType::AST_STRING;
    }
    else if(ast->ast_type_ == ASTType::AST_INTEGER){
        result.num = ast->ast_value_.num;
        result.type_ = ASTType::AST_INTEGER;
    }
    else if(ast->ast_type_ == ASTType::AST_DECIMAL){
        result.dec = ast->ast_value_.dec;
        result.type_ = ASTType::AST_DECIMAL;
    }
    else if(ast->ast_type_ == ASTType::AST_SYM){
        result = eval_symbol(ast);
        // operand.
        auto& subs = ast->sub_asts_;
        for(int i = 0; i < subs.size(); ++ i){
            ASSERT_EXIT(result.object != nullptr, "obj == nullptr");
            if(subs[i]->ast_type_ == ASTType::AST_STRING){
                std::string id = eval_exp(subs[i]->sub_asts_[0]).str;
                ASSERT_EXIT(result.object->idx_.count(id), "no such index(%s) in object(%s)\n", subs[i]->ast_value_.str.c_str(),result.str.c_str());
                int idx = result.object->idx_[id];
                ASSERT_EXIT(result.object->obj_.count(idx), "no such index(%d) in object(%s)\n", idx, result.str.c_str());
                result = result.object->obj_[idx];
            }else if(subs[i]->ast_type_ == ASTType::AST_INTEGER){
                int idx = eval_exp(subs[i]->sub_asts_[0]).num;
                ASSERT_EXIT(result.object->obj_.count(idx), "no such index(%d) in object(%s)\n", idx, result.str.c_str());
                result = result.object->obj_[idx];
            }else{
                ASSERT_EXIT(false, "unexpected ASTType(%s) as SYM's son node\n", asttype_2_string[subs[i]->ast_type_].c_str());
            }
        }
    }
    else if(ast->ast_type_ == ASTType::AST_CALL){
        result = eval_call(ast);
    }else if(ast->ast_type_ == ASTType::AST_ASSIGN){
        result = eval_assign(ast);
    }
    else if(arith_callback_.count(ast->ast_type_)){
        auto lhs = ast->eval_exp(ast->sub_asts_[0]);
        auto rhs = ast->eval_exp(ast->sub_asts_[1]);
        result = arith_callback_[ast->ast_type_](&lhs, rhs);
    }else{
        print(ast);
        ASSERT_EXIT(false, "unexpected asttype(%s)\n", asttype_2_string[ast->ast_type_].c_str());
    }
    return result;
}

Symbol AST::eval_assign(AST* ast){

    /** symbol  expression */
    auto symbol = ast->sub_asts_[0];
    auto symbol_name = symbol->ast_value_.str;
    auto val = eval_exp(ast->sub_asts_[1]);

    // FIXME: scope error, find name scope layer by layer
    Symbols * scope = nullptr;
    if(env_.current_.size() && env_.current_.back().count(symbol_name)){
        scope = &env_.current_.back();
    }
    if(scope == nullptr && env_.global_.count(symbol_name)){
        scope = &env_.global_;
    }
    if(scope == nullptr && env_.current_.size()) {
        scope = &env_.current_[env_.current_.size() - 1];
    }
    if(scope == nullptr){
        scope = &env_.global_;
    }
    Symbol* symbol_ptr = &((*scope)[symbol_name]);

    /** only object owns sub_asts */
    for(auto ast : symbol->sub_asts_){
        int32_t idx = -1;
        if(symbol_ptr->object == nullptr){
            symbol_ptr->object = new Object;
        }
        /** two way: object.name / object[index] */
        if(ast->ast_type_ == ASTType::AST_STRING){
            std::string idxname = eval_exp(ast->sub_asts_[0]).str;
            if(symbol_ptr->object->idx_.count(idxname)){
                idx = symbol_ptr->object->idx_[idxname];
            }else{
                if(symbol_ptr->object->obj_.size() == 0) idx = 0;
                else idx = (symbol_ptr->object->obj_.rbegin()->first) + 1;
            }
        }else if(ast->ast_type_ == ASTType::AST_INTEGER){
            idx = eval_exp(ast->sub_asts_[0]).num;
        }else{
            ASSERT_EXIT(false, "unexpected ASTType(%s) as SYM(%s) son node\n", asttype_2_string[ast->ast_type_].c_str(), symbol_name.c_str());
        }
        symbol_ptr = &(symbol_ptr->object->obj_[idx]);
    }
    (*symbol_ptr) = val;
    return (*symbol_ptr);
}
Symbol AST::eval_while(AST* ast){
    Symbol symbol;
    symbol.type_ = ASTType ::AST_VOID;
    auto val = eval_exp(ast->sub_asts_[0]);
    while (val.type_ == ASTType ::AST_INTEGER && val.num){
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
    symbol.type_ = ASTType ::AST_VOID;

    auto val = eval_exp(ast->sub_asts_[0]);
    bool ok = val.type_ == ASTType ::AST_INTEGER && val.num;
    ok |= val.type_ == ASTType ::AST_DECIMAL && val.dec;
    if(ok){
        auto result = interpret(ast->sub_asts_[1]);
        return result;
    }
    return symbol;
}
Symbol AST::eval_call(AST* ast){

    // built-in functions
    if(is_builtin(ast)){
        return eval_builtin(ast);
    }

    std::vector<Symbol> symbols;
    for(size_t i = 0; i < ast->sub_asts_.size(); ++ i) {
        Symbol tmp = eval_exp(ast->sub_asts_[i]);
        symbols.push_back(tmp);
    }

    // enter env ( every assign after in the scope)
    env_.current_.emplace_back();
    auto& local = env_.current_.back();
    auto fname = ast->ast_value_.str;
    ASSERT_EXIT(env_.funcs_.count(fname), "use undefined symbol(%s)", fname.c_str());
    auto f = env_.funcs_[fname];
    ASSERT_EXIT(ast->sub_asts_.size() == f.args_symbol_.size(), "number of args not same (%s)", fname.c_str());
    for(size_t i = 0; i < ast->sub_asts_.size(); ++ i) {
        local[f.args_symbol_[i]] = symbols[i];
//        local[f.args_symbol_[i]] = eval_exp(ast->sub_asts_[i]);
    }
    auto result_symbol = interpret(f.ast);

    // restore env.
    env_.current_.pop_back();
    return result_symbol;
}

Symbol AST::interpret(AST* block) {
    Symbol symbol{};
    for(auto* ast : block->sub_asts_){
        if(ast->ast_type_ == ASTType::AST_BLOCK)  symbol = interpret(ast);
        if(ast->ast_type_ == ASTType::AST_FUN)    eval_function(ast);
        if(ast->ast_type_ == ASTType::AST_ASSIGN) eval_assign(ast);
        if(ast->ast_type_ == ASTType::AST_CALL)   eval_call(ast);
        if(ast->ast_type_ == ASTType::AST_IF)     symbol = eval_if(ast);
        if(ast->ast_type_ == ASTType::AST_WHILE)  symbol = eval_while(ast);
        if(ast->ast_type_ == ASTType::AST_RETURN){
            if(!ast->sub_asts_.size()){
                symbol.type_ = ASTType ::AST_VOID;
            }else{
                symbol = eval_exp(ast->sub_asts_[0]);;
            }
            symbol.return_flag_ = 1;
            return symbol;
        }
        if(symbol.return_flag_) return symbol;
    }
    return symbol;
}


std::map<ASTType , std::function<Symbol&(Symbol *, const Symbol&)>  > arith_callback_ = {
    {ASTType ::AST_ADD,std::bind(&Symbol::operator+, std::placeholders::_1, std::placeholders::_2) },
    {ASTType ::AST_SUB,std::bind(&Symbol::operator-, std::placeholders::_1, std::placeholders::_2) },
    {ASTType ::AST_MUL,std::bind(&Symbol::operator*, std::placeholders::_1, std::placeholders::_2) },
    {ASTType ::AST_DIV,std::bind(&Symbol::operator/, std::placeholders::_1, std::placeholders::_2) },
    {ASTType ::AST_MOD,std::bind(&Symbol::operator%, std::placeholders::_1, std::placeholders::_2) },
    {ASTType ::AST_OR,std::bind(&Symbol::operator||, std::placeholders::_1, std::placeholders::_2) },
    {ASTType ::AST_AND,std::bind(&Symbol::operator&&, std::placeholders::_1, std::placeholders::_2) },
    {ASTType ::AST_BIT_OR,std::bind(&Symbol::operator|, std::placeholders::_1, std::placeholders::_2) },
    {ASTType ::AST_BIT_AND,std::bind(&Symbol::operator&, std::placeholders::_1, std::placeholders::_2) },
    {ASTType ::AST_BIT_XOR,std::bind(&Symbol::operator^, std::placeholders::_1, std::placeholders::_2) },

    {ASTType ::AST_NOT_EQUAL,std::bind(&Symbol::operator!=, std::placeholders::_1, std::placeholders::_2) },
    {ASTType ::AST_EQUAL,std::bind(&Symbol::operator==, std::placeholders::_1, std::placeholders::_2) },
    {ASTType ::AST_LARGER,std::bind(&Symbol::operator>, std::placeholders::_1, std::placeholders::_2) },
    {ASTType ::AST_LARGER_EQUAL,std::bind(&Symbol::operator>=, std::placeholders::_1, std::placeholders::_2) },
    {ASTType ::AST_LESS,std::bind(&Symbol::operator<, std::placeholders::_1, std::placeholders::_2) },
    {ASTType ::AST_LESS_EQUAL,std::bind(&Symbol::operator<=, std::placeholders::_1, std::placeholders::_2) },
    {ASTType ::AST_SHL,std::bind(&Symbol::operator<<, std::placeholders::_1, std::placeholders::_2) },
    {ASTType ::AST_SHR,std::bind(&Symbol::operator>>, std::placeholders::_1, std::placeholders::_2) },
};