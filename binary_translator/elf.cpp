#pragma pack(push, 1)
#include "tree.h"
#include <cstdlib>
#include <elf.h>
#include "elf.h"
#include <sys/mman.h>


void AST_tree::create_executable(const char* file){
    mprotect(this->jit_buffer, this->jit_buffer_size, PROT_READ | PROT_WRITE);

    remove(file);
    if(!this->jit_buffer){
        printf("Can't create executable, compile first\n");
        return;

    }

    size_t offset = 0;
    ELF_header head;
    
    printf("%lld, %lld", sizeof(ELF_header), sizeof(Programm_Header));

    Programm_Header p_head;

    p_head.FLAGS = PF_R | PF_X | PF_W;
    p_head.OFFSET = 120;
    p_head.FILESZ = this->jit_buffer_size;
    p_head.MEMSZ = this->jit_buffer_size;

    FILE* fp = fopen(file, "wb");
    
    
    fwrite(&head, sizeof(ELF_header), 1, fp);
    fwrite(&p_head, sizeof(Programm_Header), 1, fp);

    *(this->return_addr) = 0x90;

    fwrite(this->jit_buffer, this->jit_buffer_size, 1, fp);

    *(this->return_addr) = RET;

    chmod(file, 0b111101101);

    fclose(fp);

    mprotect(this->jit_buffer, this->jit_buffer_size, PROT_READ);

}
