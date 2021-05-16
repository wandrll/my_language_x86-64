#pragma once
const int register_count = 8;
enum Registers{
    RAX = 0x00,
    RCX = 0x01,
    RDX = 0x02,
    RBX = 0x03,
    RSP = 0x04,
    RBP = 0x05,
    RSI = 0x06,  
    RDI = 0x07
};

enum Stack_opcodes{
    PUSH = 0x50,
    POP  = 0x58
};

enum Call_opcodes{
    RET = 0xc3,
    CALL = 0xe8
};

enum Mov_opcodes{
    MOV_RR = 0x8948,
    MOV_MR = 0x8948,
    MOV_RM = 0x8b48,
    MOV_RI = 0x48
};

enum Add_opcodes{
    ADD_RR = 0x0148,
    ADD_RI = 0x8148  
};

enum Sub_opcodes{
    SUB_RR = 0x2948,
    SUB_RI = 0x8148
};

enum Math_opcodes{
    IDIV = 0xf748, 
    IMUL = 0xf748   //11101000
};

enum Logic_opcodes{
    XOR_RR = 0x3148
};

enum Cmp_opcodes{
    CMP_RI = 0x8148,
    CMP_RR = 0x3948
};

enum Jump_opcodes{
    JE  = 0x840f,
    JNE = 0x850f,
    JL  = 0x8c0f,
    JLE = 0x8e0f,
    JG  = 0x8f0f,
    JGE = 0x8d0f,
    JMP = 0xe9
};

// enum SSE_opcodes{
//     CVTSI2SD = 
//     CVTTSD@SI
// };
