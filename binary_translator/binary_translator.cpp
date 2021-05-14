#include "tree.h"
#include "../hash_map/hash_map.hpp"
#include <sys/mman.h>
#include <sys/stat.h>
#include <cassert>
#include <cstring>


const char* reserved_name_for_main_func = "main0";
const char* reserved_name_for_printf    = "fixed_printf";
const char* reserved_name_for_scanf     = "fixed_scanf";
const char* obj_name_io                 = "binary_translator/fixed_io.o";

const char* printf_start_label          = "PRINTF_STARTS_HERE";
size_t leng_printf_start_label = strlen(printf_start_label);

const char* printf_end_label            = "PRINTF_ENDS_HERE";
size_t leng_printf_end_label = strlen(printf_end_label);

const char* scanf_start_label          = "SCANF_STARTS_HERE";
size_t leng_scanf_start_label = strlen(scanf_start_label);

const char* scanf_end_label            = "SCANF_ENDS_HERE";
size_t leng_scanf_end_label = strlen(scanf_end_label);

static size_t file_size(const char* file){
    assert(file != NULL);

    struct stat st = {};
    stat(file, &st);
    return st.st_size;
}

size_t AST_tree::x86_load_printf(char* line){

    size_t size = file_size(obj_name_io);

    char* buffer = (char*)calloc(size+1, sizeof(char));        
    assert(buffer != NULL);

    FILE* fp = fopen (obj_name_io, "rb");
    assert(fp != NULL);
    
    fread(buffer, sizeof(char), size, fp);
    fclose(fp);

    char* curr = buffer;

    while(curr != buffer + size){
        if(strncmp(curr, printf_start_label, leng_printf_start_label) == 0){
            break;
        }    
        curr++;
    }


    assert(curr != NULL);

    char* begin = curr;

    begin += leng_printf_start_label;

    while(!(*begin)){
        begin++;
    }

    while(curr != buffer + size){

        if(strncmp(curr, printf_end_label, leng_printf_end_label) == 0){
            break;
        }    
        curr++;
    }

    assert(curr != NULL);
    char* end = curr;
    size_t length = end - begin;

    memcpy(line, begin, length);

    free(buffer);
    return length; 

}

size_t AST_tree::x86_load_scanf(char* line){

    size_t size = file_size(obj_name_io);

    char* buffer = (char*)calloc(size+1, sizeof(char));        
    assert(buffer != NULL);

    FILE* fp = fopen (obj_name_io, "rb");
    assert(fp != NULL);
    
    fread(buffer, sizeof(char), size, fp);
    fclose(fp);

    char* curr = buffer;
    while(curr != buffer + size){

        if(strncmp(curr, scanf_start_label, leng_scanf_start_label) == 0){
            break;
        }    
        curr++;
    }
    assert(curr != NULL);

    char* begin = curr;

    begin += leng_scanf_start_label;

    while(!(*begin)){
        begin++;
    }

    while(curr != buffer + size){

        if(strncmp(curr, scanf_end_label, leng_scanf_end_label) == 0){
            break;
        }    
        curr++;
    }

    assert(curr != NULL);
    char* end = curr;
    size_t length = end - begin;

    memcpy(line, begin, length);

    free(buffer);
    return length; 

}




void AST_tree::execute_JIT_compiled_buffer(){
    if(this->jit_buffer){
        void (*func)() = (void(*)())(this->jit_buffer);
        func();
    }
}


void AST_tree::JIT_compile(){
    system("nasm -f elf64 binary_translator/fixed_io.asm");


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





    offset += x86_emit_ret(buffer + offset);
    offset += x86_generate_code(buffer + offset);

    hash_map_insert(this->label_table, reserved_name_for_printf, buffer + offset);
    offset += x86_load_printf(buffer + offset);


    hash_map_insert(this->label_table, reserved_name_for_scanf, buffer + offset);
    offset += x86_load_scanf(buffer + offset);
    
    
    printf("%33sbuffer address: %p\n","", buffer);
    x86_fill_labels();  
    
    // for(int i = 0; i < offset; i++){
        // unsigned char c = buffer[i];
        // printf("\\x%02x\n", c);
    // }

    fflush(stdout);
    this->jit_buffer = buffer;

    hash_map_destructor(this->label_table);
    free(this->label_table);

    this->prts_for_free->destructor();
    free(this->prts_for_free);

    this->generated_labels->destructor();
    free(this->generated_labels);

    this->labels_to_fill->destructor();
    free(this->labels_to_fill);
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

        // case CONDITION:{
            // return nasm_generate_condition(index, line);
        // }
        // case LOOP:{
            // return nasm_generate_loop(index, line);
        // }
        // case WINDOW:{
            // printf("Unsupported operation, bitch\n");
            // fflush(stdout);
            // abort();
        // }
        // case DRAW:{
            // printf("Unsupported operation, bitch\n");
            // fflush(stdout);
            // abort();
        // }
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
            off += x86_print_binary_operator(index, line+off);
            break;
        }
/*
        case LOGIC_OP:{
            size_t offset = 0;
            off+=nasm_generate_expression(curr.right, line+off);
            
            sprintf(line + off,   "push rax\n%n", &offset);
            off+=offset;

            off+=nasm_generate_expression(curr.left, line+off);
            
            sprintf(line + off,   "pop rbx\n%n", &offset);
            off+=offset;
            
            off+=nasm_print_logical_operator(index, line+off);
            
            this->curr_label++;
            break;
        }

         case LOGIC:{
            size_t offset = 0;
            off+=nasm_generate_expression(curr.right, line+off);
            
            sprintf(line + off,   "push rax\n%n", &offset);
            off+=offset;

            off+=nasm_generate_expression(curr.left, line+off);
            
            sprintf(line + off,   "pop rbx\n%n", &offset);
            off+=offset;
            
            off+=nasm_print_logic(index, line+off);
            
            this->curr_label++;
             break;
         }
*/
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
        /*
        case PUT_PIXEL:{
            printf("Unsupported operation, bitch\n");
            fflush(stdout);
            abort();
        }
        case SIN:{
            printf("Unsupported operation, bitch\n");
            fflush(stdout);
            abort();
        }
        case COS:{
            printf("Unsupported operation, bitch\n");
            fflush(stdout);
            abort();
        }
        case SQRT:{
            offset+= nasm_generate_sqrt(curr.left, line);
        }*/
    }
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

size_t AST_tree::x86_print_binary_operator(size_t index, char* line){
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
