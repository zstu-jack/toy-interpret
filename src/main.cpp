#include <iostream>

#include "evaluate.h"
#include "define.h"
#include "utils.h"
#include "parser.h"

#if defined(__linux__)
#include <signal.h>
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

AST* blocks;

void init(){
    #if defined(__linux__)
        init_sig();
    #endif
}

int main(int argc, char* argvs[]) {

    if(argc != 2){
        std::cout << "usage: only parser file is supported for now" << std::endl;
        std::cout << "usage: binary program file (etc.. a.out example)" << std::endl;
        exit(0);
    }

    Tokenizer tokenizer;
    blocks = new AST;
    std::string program = read_from_file(argvs[1]);
    tokenizer.parse(program);

#ifdef DEBUG
    tokenizer.print_tokens();
    tokenizer.print_line_tokens();
#endif

    blocks = blocks->build(tokenizer);

#ifdef DEBUG
    blocks->print(0, blocks);
#endif

    blocks->interpret(blocks);

    return 0;
}