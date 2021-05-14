#include "tree.h"



size_t AST_tree::x86_gen_push(char* line, Registers reg){
    switch(reg){
        case RAX:{
            memcpy(line,"\x50", 1);
            return 1;
        }
        case RBX:{
            memcpy(line,"\x53", 1);
            return 1;
        }
        case RCX:{
            memcpy(line,"\x51", 1);
            return 1;
        }
        case RDX:{
            memcpy(line,"\x52", 1);
            return 1;
        }
        case RSP:{
            memcpy(line,"\x54", 1);
            return 1;
        }
        case RBP:{
            memcpy(line,"\x55", 1);
            return 1;
        }
        case RDI:{
            memcpy(line,"\x57", 1);
            return 1;
        }
        case RSI:{
            memcpy(line,"\x56", 1);
            return 1;
        }
        case R9:{
            memcpy(line,"\x41\x51", 2);
            return 2;
        }
        case R10:{
            memcpy(line,"\x41\x52", 2);
            return 2;
        }
        case R11:{
            memcpy(line,"\x41\x53", 2);
            return 2;
        }
        case R12:{
            memcpy(line,"\x41\x54", 2);
            return 2;
        }
        case R13:{
            memcpy(line,"\x41\x55", 2);
            return 2;
        }
        case R14:{
            memcpy(line,"\x41\x56", 2);
            return 2;
        }
        case R15:{
            memcpy(line,"\x41\x57", 2);
            return 2;
        }            

    }

    printf("Unknown register\n");
    assert(0);
    return -1;
}





size_t AST_tree::x86_gen_pop(char* line, Registers reg){
    switch(reg){
        case RAX:{
            memcpy(line,"\x58", 1);
            return 1;
        }
        case RBX:{
            memcpy(line,"\x5b", 1);
            return 1;
        }
        case RCX:{
            memcpy(line,"\x59", 1);
            return 1;
        }
        case RDX:{
            memcpy(line,"\x5a", 1);
            return 1;
        }
        case RSP:{
            memcpy(line,"\x5c", 1);
            return 1;
        }
        case RBP:{
            memcpy(line,"\x5d", 1);
            return 1;
        }
        case RDI:{
            memcpy(line,"\x5f", 1);
            return 1;
        }
        case RSI:{
            memcpy(line,"\x5e", 1);
            return 1;
        }
        case R9:{
            memcpy(line,"\x41\x59", 2);
            return 2;
        }
        case R10:{
            memcpy(line,"\x41\x5a", 2);
            return 2;
        }
        case R11:{
            memcpy(line,"\x41\x5b", 2);
            return 2;
        }
        case R12:{
            memcpy(line,"\x41\x5c", 2);
            return 2;
        }
        case R13:{
            memcpy(line,"\x41\x5d", 2);
            return 2;
        }
        case R14:{
            memcpy(line,"\x41\x5e", 2);
            return 2;
        }
        case R15:{
            memcpy(line,"\x41\x5f", 2);
            return 2;
        }            

    }

    printf("Unknown register\n");
    assert(0);
    return -1;
}



size_t AST_tree::x86_gen_ret(char* line){
    *line = 0xc3;
    return 1;
}

size_t AST_tree::x86_gen_call(char* line){
    *line = 0xe8;
    return 5;
}


size_t AST_tree::x86_gen_mov_rbp_rsp(char* line){ //    = mov rbp, rsp
    memcpy(line, "\x48\x89\xe5", 3);
    return 3;
}

size_t AST_tree::x86_gen_mov_rsp_rbp(char* line){ //    = mov rsp, rbp
    memcpy(line, "\x48\x89\xec", 3);
    return 3;
}

void AST_tree::fill_x_bytes(int x, long long a, char* line){
    for(int j = 0; j < x; j++){
        *(line + j) = *(((char*)(&a)) + j);
    }
}


size_t AST_tree::x86_gen_mov_var_rax(char* line, int var_offset){ //    = mov [rbp + var_offset], rax
    size_t offset = 0;
    memcpy(line, "\x48\x89\x85", 3);
    offset += 3;
                                                //    = mov [rbp + var_offset], rax
    fill_x_bytes(4, var_offset, line + offset);    //   
    offset += 4;  

    return offset;
}


size_t AST_tree::x86_gen_sub_rsp_8(char*line){//        = sub rsp, 8
    memcpy(line, "\x48\x83\xec\x08", 4);
    return 4;
}

size_t AST_tree::x86_gen_mov_rax_const(char* line, long long value){//  mov rax, number
    size_t offset = 0;

    memcpy(line + offset, "\x48\xb8", 2);
    offset += 2;

    fill_x_bytes(8, value, line + offset);
    offset += 8;

    return offset;
}

size_t AST_tree::x86_gen_mov_rdi_rax(char* line){                //mov rdi, rax
    memcpy(line, "\x48\x89\xC7", 3);
    return 3;
}