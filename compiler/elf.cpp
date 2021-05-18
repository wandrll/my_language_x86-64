#pragma pack(push, 1)
#include "compiler.h"
#include <cstdlib>
#include <elf.h>
#include "elf.h"
#include <sys/mman.h>


void Compiler::create_executable(const char* file){

    remove(file);
    if(!this->buffer){
        printf("Can't create executable, compile first\n");
        return;

    }

    size_t offset = 0;
    ELF_header head;


    Programm_Header p_head;

    p_head.FLAGS = PF_R | PF_X | PF_W;
    p_head.OFFSET = sizeof(ELF_header) + sizeof(Programm_Header);
    p_head.FILESZ = this->buffer_size;
    p_head.MEMSZ = this->buffer_size;

    FILE* fp = fopen(file, "wb");
    
    
    fwrite(&head, sizeof(ELF_header), 1, fp);
    fwrite(&p_head, sizeof(Programm_Header), 1, fp);

    fwrite(this->buffer, this->buffer_size, 1, fp);

    chmod(file, 0b111101101);

    fclose(fp);


}
