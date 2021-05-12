#include <stdio.h>
#include "tree.h"
const char* default_name = "a.asm";

int main(const int argc, const char** argv){
    if(argc == 1){
        printf("Error, there is no input file\n");
        abort();
    }

    AST_tree tree = {};
    tree.lexical_analysis(argv[1]);
    tree.build_AST();

    if(argc == 2){
        tree.create_nasm_file(default_name);
    }else{
        if(argv[2][0] == '-'){
            tree.create_nasm_file(default_name);
            tree.dump_tree("result.pdf");
        }else{
            tree.create_nasm_file(argv[2]);
            if(argc == 4 && argv[3][0] == '-'){
                tree.dump_tree("result.pdf");
            }
        }
    }

    tree.destructor();
    system("nasm -f elf64 a.asm");
    // system("nasm -f elf64 binary_translator/printf.asm");
    system("nasm -f elf64 binary_translator/fixed_io.asm");

    system("ld -s a.o binary_translator/fixed_io.o -o executable");
}
