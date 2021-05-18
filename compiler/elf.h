#pragma pack(push, 1)
#include "compiler.h"
#include <cstdlib>
#include <elf.h>

struct ELF_header{
    __uint32_t MAGIC        = 0x464C457F;
    __uint8_t  ClASS        = ELFCLASS64;
    __uint8_t  ei_DATA      = ELFDATA2LSB;          
    __uint32_t VERSION      = 0x00000001;           
    __uint16_t OSABI        = ELFOSABI_NONE;        
    __uint32_t OSABIVER     = 0x00000000;           
    __uint16_t TYPE         = ET_EXEC;              
    __uint16_t MACHINE      = EM_X86_64;            
    __uint32_t E_VERSION    = EV_CURRENT;           
    __uint64_t ENTRY        = 0x0000000000301078;   
    __uint64_t PHOFF        = 0x0000000000000040;   
    __uint64_t SHOFF        = 0x0000000000000000;   
    __uint32_t FLAGS        = 0x00000000;           
    __uint16_t EHSIZE       = 0x0040;               
    __uint16_t PHENTSIZE    = 0x0038;               
    __uint16_t PHNUM        = 0x0001;               
    __uint16_t SHENTSIZE    = 0x0000;               
    __uint16_t SHNUM        = 0x0000;               
    __uint16_t SHSTRNDX     = 0x0000;   
};

struct Programm_Header {
    __uint32_t TYPE         = 0x00000000 | PT_LOAD;       
    __uint32_t FLAGS        = 0x00000000;          
    __uint64_t OFFSET       = 0x0000000000000000;         
    __uint64_t VADDR        = 0x0000000000301078;         
    __uint64_t PADDR        = 0x0000000000301078;         
    __uint64_t FILESZ       = 0x0000000000000080;         
    __uint64_t MEMSZ        = 0x0000000000000080;         
    __uint64_t ALIGN        = 0x0000000000000001;         
};


