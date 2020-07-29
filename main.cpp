#include <iostream>

#include "evaluate.h"
#include "define.h"
#include "utils.h"
#include "parser.h"

AST* blocks;
Tokenizer* tokenizer;

void init(){
    blocks = new AST;
    tokenizer = new Tokenizer;
}

int main(int argc, char* argvs[]) {

    init();

    if(argc == 1){
        std::string statement;
        for(;;){
            std::cin >> statement;
            // TODO
        }
        // interrupt.
    }

    ASSERT_EXIT(argc == 2, "only one file supported for now");
    std::string program = read_from_file(argvs[1]);
    tokenizer->parse(program);
    //tokenizer->print();
    blocks = blocks->build(tokenizer->tokens_);
    //blocks->print(0, blocks);
    blocks->interpret(blocks);

    return 0;
}