#include <stdio.h>
#include "compiler.h"
const char* default_name = "res";

int main(const int argc, const char** argv){
    if(argc == 1){
        printf("Error, there is no input file\n");
        abort();
    }

    Compiler comp = {};
    comp.constructor();

    if(argc == 2){
        comp.compile(argv[1], default_name);

    }else{
        if(argv[2][0] == '-'){
            comp.dump_tree("result.pdf");

            comp.compile(argv[1], default_name);


        }else{
            if(argc == 4 && argv[3][0] == '-'){
                comp.dump_tree("result.pdf");
            }

            comp.compile(argv[1], argv[2]);


        }
    }

    // tree.create_executable("res");

    // tree.execute_JIT_compiled_buffer();


    comp.destructor();

}
