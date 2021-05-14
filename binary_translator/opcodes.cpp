#include "tree.h"



size_t AST_tree::x86_emit_push(char* line, Registers reg){
    assert(reg < register_count);

    *line = (PUSH + reg);
    return 1;
}



size_t AST_tree::x86_emit_pop(char* line, Registers reg){
    assert(reg < register_count);

    *line = (POP + reg);
    return 1;
}



size_t AST_tree::x86_emit_ret(char* line){
    *line = RET;
    return 1;
}

size_t AST_tree::x86_emit_call(char* line){
    *line = CALL;
    return 5;
}

size_t AST_tree::x86_emit_mov_r64_r64(char* line, Registers reg1, Registers reg2){
    assert(reg1 < register_count && reg2 < register_count);

    u_int16_t mov_op = MOV_RR;
    
    * line      = *(( u_int8_t*)(&mov_op));
    *(line + 1) = *(((u_int8_t*)(&mov_op)) + 1);


    u_int8_t byte = (0b11000000) | reg1 | (reg2 << 3);

    *(line + 2) = byte;

    return 3;
}



void AST_tree::fill_x_bytes(int x, int64_t a, char* line){
    for(int j = 0; j < x; j++){
        *(line + j) = *(((char*)(&a)) + j);
    }
}


size_t AST_tree::x86_emit_mov_mem_r64(char* line, Registers mem_reg, int32_t mem_off, Registers src_reg){
    assert(mem_reg < register_count && src_reg < register_count);

    u_int16_t mov_op = MOV_MR;
    
    * line      = *(( u_int8_t*)(&mov_op));
    *(line + 1) = *(((u_int8_t*)(&mov_op)) + 1);


    u_int8_t byte = (0b10000000) | mem_reg | (src_reg << 3);

    *(line + 2) = byte;

    fill_x_bytes(4, mem_off, line + 3);

    return 7;

}


size_t AST_tree::x86_emit_mov_r64_mem(char* line,  Registers src_reg, Registers mem_reg, int32_t mem_off){
    assert(mem_reg < register_count && src_reg < register_count);

    u_int16_t mov_op = MOV_RM;
    
    * line      = *(( u_int8_t*)(&mov_op));
    *(line + 1) = *(((u_int8_t*)(&mov_op)) + 1);


    u_int8_t byte = (0b10000000) | mem_reg | (src_reg << 3);

    *(line + 2) = byte;

    fill_x_bytes(4, mem_off, line + 3);

    return 7;

}

size_t AST_tree::x86_emit_mov_r64_imm(char* line, Registers reg, int64_t value){
    assert(reg < register_count);

    
    u_int8_t mov_op = MOV_RI;
    
    * line      = *(( u_int8_t*)(&mov_op));

    u_int8_t byte = (0b10111000) | reg ;

    *(line + 1) = byte;

    fill_x_bytes(8, value, line + 2);

    return 10;
}

size_t AST_tree::x86_emit_add_r64_r64(char* line, Registers reg1, Registers reg2){
    assert(reg1 < register_count && reg2 < register_count);

    u_int16_t mov_op = ADD_RR;
    
    * line      = *(( u_int8_t*)(&mov_op));
    *(line + 1) = *(((u_int8_t*)(&mov_op)) + 1);


    u_int8_t byte = (0b11000000) | reg1 | (reg2 << 3);

    *(line + 2) = byte;

    return 3;
}


size_t AST_tree::x86_emit_add_r64_imm(char* line, Registers reg, int32_t value){
    assert(reg < register_count);

    
    u_int16_t mov_op = ADD_RI;
    
    * line      = *(( u_int8_t*)(&mov_op));
    *(line + 1) = *(((u_int8_t*)(&mov_op)) + 1);

    u_int8_t byte = (0b11000000) | reg ;

    *(line + 2) = byte;

    fill_x_bytes(4, value, line + 3);

    return 7;
}


size_t AST_tree::x86_emit_sub_r64_r64(char* line, Registers reg1, Registers reg2){
    assert(reg1 < register_count && reg2 < register_count);

    u_int16_t mov_op = SUB_RR;
    
    * line      = *(( u_int8_t*)(&mov_op));
    *(line + 1) = *(((u_int8_t*)(&mov_op)) + 1);


    u_int8_t byte = (0b11000000) | reg1 | (reg2 << 3);

    *(line + 2) = byte;

    return 3;
}



size_t AST_tree::x86_emit_sub_r64_imm(char* line, Registers reg1, int32_t value){
    assert(reg1 < register_count);

    u_int16_t mov_op = SUB_RI;
    
    * line      = *(( u_int8_t*)(&mov_op));
    *(line + 1) = *(((u_int8_t*)(&mov_op)) + 1);


    u_int8_t byte = (0b11101000) | reg1 ;

    *(line + 2) = byte;

    fill_x_bytes(4, value, line + 3);

    return 7;
}



size_t AST_tree::x86_emit_xor_r64_r64(char* line, Registers reg1, Registers reg2){
    assert(reg1 < register_count && reg2 < register_count);

    u_int16_t mov_op = XOR_RR;
    
    * line      = *(( u_int8_t*)(&mov_op));
    *(line + 1) = *(((u_int8_t*)(&mov_op)) + 1);


    u_int8_t byte = (0b11000000) | reg1 | (reg2 << 3);

    *(line + 2) = byte;

    return 3;     
}


size_t AST_tree::x86_emit_imul_r64(char* line, Registers reg1){
    assert(reg1 < register_count);

    u_int16_t mov_op = IMUL;
    
    * line      = *(( u_int8_t*)(&mov_op));
    *(line + 1) = *(((u_int8_t*)(&mov_op)) + 1);


    u_int8_t byte = (0b11101000) | reg1 ;

    *(line + 2) = byte;


    return 3;
}



size_t AST_tree::x86_emit_idiv_r64(char* line, Registers reg1){
    assert(reg1 < register_count);

    u_int16_t mov_op = IDIV;
    
    * line      = *(( u_int8_t*)(&mov_op));
    *(line + 1) = *(((u_int8_t*)(&mov_op)) + 1);


    u_int8_t byte = (0b11111000) | reg1 ;

    *(line + 2) = byte;


    return 3;
}

