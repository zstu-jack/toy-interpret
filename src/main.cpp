#include <iostream>

#include "evaluate.h"
#include "define.h"
#include "utils.h"
#include "parser.h"

#if defined(__linux__)
#include <signal.h>
#endif

AST* blocks;
Tokenizer* tokenizer;

#if defined(__linux__)
void sigint_handle(int sig_val)
{
    fprintf(stdout, "receive SIG_INT, quit the program\n");
    fflush(stdout);
    exit(0);
}

void init_sig()
{
    signal(SIGINT, sigint_handle);
}
#endif

void init(){

    blocks = new AST;
    tokenizer = new Tokenizer;
}

int main(int argc, char* argvs[]) {
#if defined(__linux__)
    init_sig();
#endif

    init();

    if(argc == 1){
        std::string statement;
        for(;;){
            std::cin >> statement;
        }
        // interrupt.
    }

    ASSERT_EXIT(argc == 2, "only one file supported for now");
    std::string program = read_from_file(argvs[1]);
    tokenizer->parse(program);
    blocks = blocks->build(tokenizer->tokens_);
#ifdef DEBUG
    tokenizer->print();
    blocks->print(0, blocks);
#endif
    blocks->interpret(blocks);

    return 0;
}