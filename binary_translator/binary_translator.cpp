#include "tree.h"
#include "../hash_map/hash_map.hpp"
#include <sys/mman.h>
#include <sys/stat.h>
#include <cassert>
#include <cstring>
#include <elf.h>
#include "opcodes.h"
#include <ctime>
#pragma pack(push, 1)

const char* reserved_name_for_main_func = "main0";
const char* reserved_name_for_printf    = "fixed_printf";
const char* reserved_name_for_scanf     = "fixed_scanf";
const char* reserved_name_for_sqrt      = "sqrt";

const char* obj_name_std                = "binary_translator/std.o";


const char* jump_table_label           = "JUMP_TABLE_BEGINS";
const size_t leng_jump_table_label = strlen(jump_table_label);





static size_t file_size(const char* file){
    assert(file != NULL);

    struct stat st = {};
    stat(file, &st);
    return st.st_size;
}


size_t AST_tree::x86_load_std(char* line){
    size_t size = file_size(obj_name_std);

    char* buffer = (char*)calloc(size+1, sizeof(char));        
    assert(buffer != NULL);

    FILE* fp = fopen (obj_name_std, "rb");
    assert(fp != NULL);
    
    fread(buffer, sizeof(char), size, fp);
    fclose(fp);

    char* curr = buffer;

    while(curr != buffer + size){
        if(memcmp(curr, jump_table_label, leng_jump_table_label) == 0){
            break;
        }    
        curr++;
    }

    curr += leng_jump_table_label;

   char* scanf_addr = *((long long*)curr) + curr;
    curr += 8;
    long long scanf_len = *((long long*)curr);
    curr += 8;


    char* printf_addr = *((long long*)curr) + curr;
    curr += 8;
    long long printf_len = *((long long*)curr);
    curr += 8;

 
    char* sqrt_addr = *((long long*)curr) + curr;
    curr += 8;
    long long sqrt_len = *((long long*)curr);
    curr += 8;

    
    size_t offset = 0;


    hash_map_insert(this->label_table, reserved_name_for_scanf, line + offset);
    memcpy(line + offset, scanf_addr, scanf_len);
    offset += scanf_len;


    hash_map_insert(this->label_table, reserved_name_for_printf, line + offset);
    memcpy(line + offset, printf_addr, printf_len);
    offset += printf_len;



    hash_map_insert(this->label_table, reserved_name_for_sqrt, line + offset);
    memcpy(line + offset, sqrt_addr, sqrt_len);
    offset += sqrt_len;

    free(buffer);

    return offset;

}


void AST_tree::execute_JIT_compiled_buffer(){
    if(this->jit_buffer){
        mprotect(this->jit_buffer, this->jit_buffer_size, PROT_READ | PROT_EXEC);

        clock_t begin = clock();
        void (*func)() = (void(*)())(this->jit_buffer);
        func();
        double time = clock() - begin;

        printf("\nExecuted successfully (%lg seconds)", time/CLOCKS_PER_SEC);

        mprotect(this->jit_buffer, this->jit_buffer_size, PROT_READ);

    }
}


void AST_tree::JIT_compile(){
    system("nasm -f elf64 binary_translator/std.asm");


    this->labels_to_fill = (List<label_pair>*)calloc(1, sizeof(List<label_pair>));
    this->labels_to_fill->constructor();

    this->generated_labels = (List<char*>*)calloc(1, sizeof(List<char*>));
    this->generated_labels->constructor();


    this->prts_for_free = (List<char*>*)calloc(1, sizeof(List<char*>));
    this->prts_for_free->constructor();

    this->label_table = (Hash_map*)calloc(1, sizeof(Hash_map));
    hash_map_constructor(this->label_table);

    char* buffer = (char*)valloc(1024 * 1024);
    mprotect(buffer, 1024 * 1024, PROT_READ | PROT_WRITE | PROT_EXEC);

    size_t offset = 0;
    size_t off = 0;


    offset += x86_emit_push(buffer + offset, RBX);

    this->labels_to_fill->push_back({reserved_name_for_main_func, buffer + offset + 1, 4});
    offset += x86_emit_call(buffer + offset);

    offset += x86_emit_pop(buffer + offset, RBX);



    this->return_addr = buffer + offset;

    offset += x86_emit_ret(buffer + offset);



    offset += x86_emit_mov_r64_imm(buffer + offset, RAX, 0x3c);
    offset += x86_emit_xor_r64_r64(buffer + offset, RDI, RDI);
    offset += x86_emit_syscall(buffer + offset);

    offset += x86_generate_code(buffer + offset);

    offset += x86_load_std(buffer + offset);

    
    printf("%33sbuffer address: %p\n","", buffer);
    x86_fill_labels();  
    
    // for(int i = 0; i < offset; i++){
        // unsigned char c = buffer[i];
        // printf("\\x%02x\n", c);
    // }

    fflush(stdout);
    if(this->jit_buffer){
        free(this->jit_buffer);
    }

    this->jit_buffer = buffer;
    this->jit_buffer_size = offset;

    hash_map_destructor(this->label_table);
    free(this->label_table);

    this->prts_for_free->destructor();
    free(this->prts_for_free);

    this->generated_labels->destructor();
    free(this->generated_labels);

    this->labels_to_fill->destructor();
    free(this->labels_to_fill);

    mprotect(buffer, this->jit_buffer_size, PROT_READ);

}




void AST_tree::x86_fill_labels(){
    size_t count = this->labels_to_fill->size;

    label_pair curr = {};
    
    for(int i = 0; i < count; i++){
        curr = this->labels_to_fill->pop_back();
        char* label_address = hash_map_get(this->label_table, curr.label);
        printf("%30s: address to fill- %p,  label - address %p\n",curr.label, curr.RIP, label_address);

        int offset =  label_address - (curr.RIP + curr.op_code_size);

        fill_x_bytes(4, offset, curr.RIP);


    }
    fflush(stdout);

    count = this->prts_for_free->size;

    for(int i = 0; i < count; i++){
        free(this->prts_for_free->pop_back());
    }

}



size_t AST_tree::x86_generate_code(char* line){
    this->scope = New_pages_list(); 

    size_t curr = this->root;
    size_t offset = 0;

    while(curr != 0){
        Tree_Node tmp = get_node(curr);
        offset+= x86_generate_function_code(tmp.left, line + offset);
        curr = tmp.right;
    };

    Delete_pages_list(this->scope);
    this->scope = NULL;
    return offset;
}



void AST_tree::x86_generate_label(AST_tree::Tree_Node node, char* line){
        size_t argc = nasm_get_func_argc(node.left);
        size_t offset = 0;
        char* label = (char*)calloc(256, sizeof(char));

        sprintf(label,"%s%ld", node.u.line, argc);
        
        if(!hash_map_insert(this->label_table, label, line)){
            printf("Error: redeclaration of label %s", label);
            fflush(stdout);
            abort();
        }

        free(label);

}

size_t AST_tree::x86_generate_default_return(char* line){
    return x86_emit_ret(line);
}


void AST_tree::x86_get_function_arguments(size_t index){
    size_t curr = index;
    while(curr != 0){
        Tree_Node Link = get_node(curr);
        Tree_Node tmp = get_node(Link.left);
        this->scope->add_arg(tmp.u.line);
        curr = Link.right;
    }
}




size_t AST_tree::x86_generate_function_code(size_t index, char* line){
    this->scope->new_scope();

    Tree_Node func = get_node(index);
    size_t offset = 0;

    x86_generate_label(func, line+offset);
    x86_get_function_arguments(func.left);

    offset+=x86_generate_body_code(func.right, line+offset);
    offset+=x86_generate_default_return(line+offset);

    this->scope->leave_scope();
    return offset;
}


size_t AST_tree::x86_generate_body_code(size_t index, char* line){
    
    Tree_Node func = get_node(index);
    size_t offset = 0;
    size_t off = 0;

    offset += x86_emit_push       (line + offset, RBP);
    offset += x86_emit_mov_r64_r64(line + offset, RBP, RSP);

    size_t curr = index;
    while(curr != 0){
        Tree_Node tmp = get_node(curr);
        offset+= x86_generate_statement_code(tmp.left, line + offset);
        curr = tmp.right;
    }

    offset += x86_emit_mov_r64_r64(line + offset, RSP, RBP);
    offset += x86_emit_pop        (line + offset, RBP);

    return offset;
}


size_t AST_tree::x86_generate_statement_code(size_t index, char* line){
    Tree_Node node = get_node(index);
    size_t offset = 0;
    switch(node.type){
        case ASSIGNMENT:{
            return x86_generate_assignment(index, line);
        }

        case RETURN:{
            return x86_generate_return(index, line);
        }

        case VARIABLE_DECLARATOR:{
            return x86_generate_variable_declaration(index, line);
        }

        case CONDITION:{
            return x86_generate_condition(index, line);
        }
        case LOOP:{
            return x86_generate_loop(index, line);
        }

        default:{
            return x86_generate_expression(index, line); 
        }
    }
}


size_t AST_tree::x86_generate_assignment(size_t index, char* line){
    Tree_Node node = get_node(index);
    size_t offset = 0;
    
    offset += x86_generate_expression(node.right, line + offset);
    
    Tree_Node tmp = get_node(node.left);
    int var = this->scope->var_offset(tmp.u.line);
    
    offset += x86_emit_mov_mem_r64(line + offset, RBP, var, RAX);
    
    return offset;
}

size_t AST_tree::x86_generate_variable_declaration(size_t index, char* line){
    Tree_Node curr = get_node(index);
    this->scope->add_var(curr.u.line);
    size_t offset = 0;
    
    if(curr.left != 0){
        offset += x86_generate_expression(curr.left, line);

        int var_off = this->scope->var_offset(curr.u.line);

        offset += x86_emit_mov_mem_r64(line + offset, RBP, var_off, RAX);
    }

    offset += x86_emit_sub_r64_imm(line + offset, RSP, 8);

    return offset;
}


size_t AST_tree::x86_generate_expression(size_t index, char* line){
    size_t off = 0;
    Tree_Node curr = get_node(index);
     switch(curr.type){
        case NUMBER:{            
            off += x86_emit_mov_r64_imm     (line + off, RAX, curr.u.value);
            break;
        }
        case BINARY_OP:{
            
            off += x86_generate_expression  (curr.right, line+off);
            off += x86_emit_push            (line + off, RAX);
            off += x86_generate_expression  (curr.left, line+off);
            off += x86_emit_pop             (line + off, RBX);
            off += x86_generate_binary_operator(index, line+off);
            break;
        }

        case LOGIC_OP:{

            off += x86_generate_expression(curr.right, line+off);
            
            off += x86_emit_push(line + off, RAX);

            off += x86_generate_expression(curr.left, line+off);
            

            off += x86_emit_pop(line + off, RBX);

            off += x86_generate_logical_operator(index, line+off);
            
         
  
            break;
        }

         case LOGIC:{
            size_t offset = 0;
            off += x86_generate_expression(curr.right, line+off);
            
            off += x86_emit_push(line + off, RAX);

            off += x86_generate_expression(curr.left, line+off);
            

            off += x86_emit_pop(line + off, RBX);

            off += x86_generate_logic(index, line+off);
            
         
             break;
         }

         case VARIABLE:{
            size_t var = this->scope->var_offset(curr.u.line);
            
            off += x86_emit_mov_r64_mem(line + off, RAX, RBP, var);
            break;
         }

         case READ:{
            this->labels_to_fill->push_back({reserved_name_for_scanf, line + off + 1, 4});
            off += x86_emit_call(line + off);
            
            break;
         }

          case STANDART_FUNCTION:{
              off += x86_generate_standart_function(index, line);
              return off;
              break;
          }

         case FUNCTION_CALL:{
             off += x86_generate_function_call(index, line);
             break;
         }

    }

    return off;
}

size_t AST_tree::x86_generate_standart_function(size_t index, char* line){
    Tree_Node curr = get_node(index);
    size_t offset = 0;
    switch((Node_type)curr.u.value){
        case WRITE:{
            offset+= x86_generate_write(curr.left, line);
            break;
        }

        case SQRT:{
            offset+= x86_generate_sqrt(curr.left, line);
        }
    }
    return offset;
}


size_t AST_tree::x86_generate_sqrt(size_t index, char* line){
    size_t offset = 0;
    size_t argc = nasm_get_func_argc(index);
    if(argc != 1){
        printf("Wrong count of sqrt arguments");
        fflush(stdout);
        assert(0);
    }
    offset+= x86_generate_expression(get_node(index).left, line);

    this->labels_to_fill->push_back({reserved_name_for_sqrt, line + offset + 1, 4});
    offset += x86_emit_call       (line + offset);

    return offset;
}

size_t AST_tree::x86_generate_write(size_t index, char* line){
    size_t count = 0;
    size_t offset = 0;
    size_t off = 0;

    while(index != 0){
        Tree_Node curr = get_node(index);
        offset += x86_generate_expression(curr.left, line+offset);
        
        offset += x86_emit_mov_r64_r64(line + offset, RDI, RAX);
        this->labels_to_fill->push_back({reserved_name_for_printf, line + offset + 1, 4});
        offset += x86_emit_call       (line + offset);   

        
        index = curr.right;

    }

    return offset;
}

size_t AST_tree::x86_generate_binary_operator(size_t index, char* line){
    size_t off = 0;
    Tree_Node curr = get_node(index);

    switch((Math_operators)curr.u.value){
        case PLUS:{
            return x86_emit_add_r64_r64(line, RAX, RBX);
        }
        case MINUS:{
            return x86_emit_sub_r64_r64(line , RAX, RBX);
        }
        case MUL:{

            off += x86_emit_mov_r64_imm(line + off, RCX, maximum_of_fractional_part);
            off += x86_emit_xor_r64_r64(line + off, RDX, RDX);
            off += x86_emit_imul_r64   (line + off, RBX);
            off += x86_emit_idiv_r64   (line + off, RCX);

            return off;
        }
        
        
        case DIV:{

            off += x86_emit_mov_r64_imm(line + off, RCX, maximum_of_fractional_part);
            off += x86_emit_xor_r64_r64(line + off, RDX, RDX);
            off += x86_emit_imul_r64   (line + off, RCX);            
            off += x86_emit_idiv_r64   (line + off, RBX);

            return off;
        }


        
    }
    printf("Error. Unknown node %d", curr.type);
    fflush(stdout);
    assert(0);
}



size_t AST_tree::x86_generate_return(size_t index, char* line){
    size_t offset = 0;
    Tree_Node curr = get_node(index);
    
    offset += x86_generate_expression(curr.left, line);

    offset += x86_emit_mov_r64_r64(line + offset, RSP, RBP);
    offset += x86_emit_pop        (line + offset, RBP);
    offset += x86_emit_ret        (line + offset);

    return offset;
}

void AST_tree::x86_generate_label_call(AST_tree::Tree_Node node, char* line){
        size_t argc = nasm_get_func_argc(node.left);
        size_t offset = 0;
        char* label = (char*)calloc(256, sizeof(char));

        sprintf(label,"%s%ld", node.u.line, argc);

        this->labels_to_fill->push_back({label, line + 1, 4});

        this->prts_for_free->push_back(label);
}

size_t AST_tree::x86_generate_func_arguments(size_t index, char* line){
    
    size_t offset = 0;

    while(index != 0){
        Tree_Node curr = get_node(index);

        offset+=x86_generate_expression(curr.left, line+offset);
        size_t off = 0;

        offset += x86_emit_push(line + offset, RAX);

        index = curr.right;
    }

    return offset;
}


size_t AST_tree::x86_generate_function_call(size_t index, char* line){
    Tree_Node curr = get_node(index); 
    size_t offset = 0;

    offset+= x86_generate_func_arguments(curr.left, line+offset);

    x86_generate_label_call(curr, line+offset);
    offset += x86_emit_call(line + offset);   

    size_t index2 = curr.left;

    int count = 0;

    while(index2 != 0){
        Tree_Node curr = get_node(index2);

        count++;
        index2 = curr.right;
    }

    offset += x86_emit_add_r64_imm(line + offset, RSP, count*8);

    return offset;

}



size_t AST_tree::x86_generate_logic(size_t index, char* line){
    size_t off = 0;
    Tree_Node curr = get_node(index);

    switch((Math_operators)curr.u.value){
        case AND:{

            off += x86_emit_cmp_r64_imm (line + off, RAX, 0);
            char* addr1_false_to_fill = line + off + 2;
            off += x86_emit_jxx         (line + off, JE);

            off += x86_emit_cmp_r64_imm (line + off, RBX, 0);
            char* addr2_false_to_fill = line + off + 2;
            off += x86_emit_jxx         (line + off, JE);

            off += x86_emit_mov_r64_imm (line + off, RAX, maximum_of_fractional_part);
            char* addr1_true_fill = line + off + 1;
            off += x86_emit_jmp         (line + off);

            fill_x_bytes(4, (line + off) - (addr1_false_to_fill + 4), addr1_false_to_fill);
            fill_x_bytes(4, (line + off) - (addr2_false_to_fill + 4), addr2_false_to_fill);

            off += x86_emit_xor_r64_r64(line + off, RAX, RAX);

            fill_x_bytes(4, (line + off) - (addr1_true_fill + 4), addr1_true_fill);

return off;
        }
        case OR:{
            off += x86_emit_cmp_r64_imm (line + off, RAX, 0);
            char* addr1_true_to_fill = line + off + 2;
            off += x86_emit_jxx         (line + off, JNE);

            off += x86_emit_cmp_r64_imm (line + off, RBX, 0);
            char* addr2_true_to_fill = line + off + 2;
            off += x86_emit_jxx         (line + off, JNE);

            off += x86_emit_xor_r64_r64(line + off, RAX, RAX);

            char* addr1_false_to_fill = line + off + 1;
            off += x86_emit_jmp         (line + off);

            fill_x_bytes(4, (line + off) - (addr1_true_to_fill + 4), addr1_true_to_fill);
            fill_x_bytes(4, (line + off) - (addr2_true_to_fill + 4), addr2_true_to_fill);
            off += x86_emit_mov_r64_imm (line + off, RAX, maximum_of_fractional_part);

            fill_x_bytes(4, (line + off) - (addr1_false_to_fill + 4), addr1_false_to_fill);


            
            return off;
        }
    }
     printf("Error. Unknown node %d", curr.type);
    fflush(stdout);
    assert(0);
}

size_t AST_tree::x86_generate_logical_operator(size_t index, char* line){
    size_t off = 0;
    Tree_Node curr = get_node(index);

    Jump_opcodes cmp_type = JE;

    switch((Math_operators)curr.u.value){
        case EQUAL:{
            cmp_type = JE;
            break;
        }
        case NOT_EQUAL:{
            cmp_type = JNE;
            break;
        }
        case BELOW:{
            cmp_type = JL;
            break;
        }
        case BELOW_EQUAL:{
            cmp_type = JLE;
            break;
        }
        case ABOVE:{
            cmp_type = JG;
            break;
        }
        case ABOVE_EQUAL:{
            cmp_type = JGE;
            break;
        }
        default:{
            printf("Error. Unknown node %d", curr.type);
            fflush(stdout);
            assert(0);
        }
    }
    off += x86_emit_cmp_r64_r64(line + off, RAX, RBX);
    char* addr_to_fill_true = line + off + 2;
    off += x86_emit_jxx(line + off, cmp_type); 

    off += x86_emit_xor_r64_r64(line + off, RAX, RAX);
    char* addr_to_fill_false = line + off + 1;

    off += x86_emit_jmp(line + off);

    fill_x_bytes(4, (line + off) - (addr_to_fill_true + 4), addr_to_fill_true);

    off += x86_emit_mov_r64_imm(line + off, RAX, maximum_of_fractional_part);

    fill_x_bytes(4, (line + off) - (addr_to_fill_false + 4), addr_to_fill_false);
    
    return off;

    
}

size_t AST_tree::x86_generate_body(size_t index, char* line){

    size_t offset = 0;
    while(index != 0){
        Tree_Node curr = get_node(index);
        offset+=x86_generate_statement_code(curr.left, line+offset);
        index = curr.right;
    }

    return offset;
}



size_t AST_tree::x86_generate_condition(size_t index, char* line){
     Tree_Node curr = get_node(index);
    
    size_t offset = 0;

    Tree_Node right = get_node(curr.right);

    offset += x86_generate_expression(curr.left, line+offset);
    offset += x86_emit_cmp_r64_imm(line + offset, RAX, 0);
    char* addr_cnd_to_fill = line + offset + 2;
    offset += x86_emit_jxx(line + offset, JE);

    offset += x86_generate_body(right.left, line+offset);
    
    char* addr_end_to_fill = line + offset + 1;    
    offset += x86_emit_jmp(line + offset);


    fill_x_bytes(4, (line + offset) - (addr_cnd_to_fill + 4), addr_cnd_to_fill);

    offset +=x86_generate_body(right.right, line+offset);

    fill_x_bytes(4, (line + offset) - (addr_end_to_fill + 4), addr_end_to_fill);

 
    return offset;
}



size_t AST_tree::x86_generate_loop(size_t index, char* line){
    Tree_Node curr = get_node(index);
    size_t offset = 0;

    char* begin = line;
    offset += x86_generate_expression(curr.left, line+offset);

    offset += x86_emit_cmp_r64_imm(line + offset, RAX, 0);

    char* addr_end_to_fill = line + offset + 2;

    offset += x86_emit_jxx(line + offset, JE);

    offset += x86_generate_body(index, line + offset);

    offset += x86_emit_jmp(line + offset);

    fill_x_bytes(4, begin - (line + offset), line + offset - 4);

    fill_x_bytes(4, (line + offset) - (addr_end_to_fill + 4), addr_end_to_fill);


    return offset;
}   
