#include "tree.h"



void AST_tree::create_nasm_file(const char* file){
    this->curr_label = 0;

    FILE* fp = fopen(file, "w");
    char* buffer = (char*)calloc(max_buffer_size+1, sizeof(char));
    size_t offset = 0;
    sprintf(buffer,"default rel\n"
                   "\n"
                   "\n"
                   "extern fixed_printf\n"
                   "extern fixed_scanf\n"

                   "\n"
                   "section .text\n"
                   "\n"
                   "global _start\n"
                   "_start:\n"
                   "call main0\n"
                   "\n"
                   "mov rax, 0x3C\n"
                   "xor rdi, rdi\n"
                   "syscall\n"

                   "%n",&offset);

    size_t size = nasm_generate_code(buffer+offset);
    size = size + offset;

    fwrite(buffer,size, sizeof(char), fp);
    fclose(fp);
    free(buffer);


}


size_t AST_tree::nasm_generate_code(char* line){
    this->scope = New_pages_list(); 

    size_t curr = this->root;
    size_t offset = 0;

    while(curr != 0){
        Tree_Node tmp = get_node(curr);
        offset+= nasm_generate_function_code(tmp.left, line + offset);
        curr = tmp.right;
    };

    Delete_pages_list(this->scope);
    this->scope = NULL;
    return offset;
}



size_t AST_tree::nasm_generate_label(AST_tree::Tree_Node node, char* line){
        size_t argc = nasm_get_func_argc(node.left);
        size_t offset = 0;
        sprintf(line,"%s%ld%n", node.u.line, argc, &offset);

        return offset;
}

size_t AST_tree::nasm_generate_default_return(char* line){
    size_t offset = 0;
    sprintf(line,"\n"
                 "ret\n\n%n",&offset);
    return offset;
}




size_t AST_tree::nasm_generate_function_code(size_t index, char* line){
    this->scope->new_scope();

    Tree_Node func = get_node(index);
    size_t offset = 0;

    offset+=nasm_generate_label(func, line+offset);

    size_t off = 0;
    sprintf(line+offset,":\n\n%n", &off);
    offset+= off;


    nasm_get_function_arguments(func.left);
    offset+=nasm_generate_body_code(func.right, line+offset);
    offset+=nasm_generate_default_return(line+offset);

    this->scope->leave_scope();
    return offset;
}

void AST_tree::nasm_get_function_arguments(size_t index){

    size_t curr = index;
    while(curr != 0){
        Tree_Node Link = get_node(curr);
        Tree_Node tmp = get_node(Link.left);
        this->scope->add_arg(tmp.u.line);
        curr = Link.right;
    }
}

size_t AST_tree::nasm_generate_body_code(size_t index, char* line){
    Tree_Node func = get_node(index);
    size_t offset = 0;
    size_t off = 0;
    sprintf(line+offset,    "push rbp\n"
                            "mov rbp, rsp\n"
                            "\n\n%n", &off);
    offset += off;

    size_t curr = index;
    while(curr != 0){
        Tree_Node tmp = get_node(curr);
        offset+= nasm_generate_statement_code(tmp.left, line + offset);
        curr = tmp.right;
    }

    sprintf(line+offset,    "\n"
                            "mov rsp, rbp\n"
                            "pop rbp\n"
                            "%n", &off);
    offset += off;

    return offset;
}


size_t AST_tree::nasm_generate_statement_code(size_t index, char* line){
    Tree_Node node = get_node(index);
    size_t offset = 0;
    switch(node.type){
        case ASSIGNMENT:{
            return nasm_generate_assignment(index, line);
        }
        case RETURN:{
            return nasm_generate_return(index, line);
        }
        case VARIABLE_DECLARATOR:{
            return nasm_generate_variable_declaration(index, line);
        }
        case CONDITION:{
            return nasm_generate_condition(index, line);
        }
        case LOOP:{
            return nasm_generate_loop(index, line);
        }
        case WINDOW:{
            printf("Unsupported operation, bitch\n");
            fflush(stdout);
            abort();
        }
        case DRAW:{
            printf("Unsupported operation, bitch\n");
            fflush(stdout);
            abort();
        }
        default:{
            return nasm_generate_expression(index, line); 
        }
    }
}


size_t AST_tree::nasm_generate_assignment(size_t index, char* line){
    Tree_Node node = get_node(index);
    size_t offset = 0;
    size_t off = 0;
    
    offset+=off;
    offset += nasm_generate_expression(node.right, line + offset);
    
    Tree_Node tmp = get_node(node.left);
    int var = this->scope->var_offset(tmp.u.line);
    
    sprintf(line + offset,  "mov [rbp + %d], rax\n"
                            "%n", var, &off);
    offset+=off;
    sprintf(line + offset,"\n");
    offset++;
    return offset;
}

size_t AST_tree::nasm_generate_variable_declaration(size_t index, char* line){
    Tree_Node curr = get_node(index);
    this->scope->add_var(curr.u.line);
    size_t offset = 0;
    
    if(curr.left != 0){
        offset+=nasm_generate_expression(curr.left, line);
    
        size_t off = 0;
        sprintf(line + offset, "mov [rbp + %d], rax\n"
                               "sub rsp, 8\n"
                               "%n",  this->scope->var_offset(curr.u.line), &off);
        offset+=off;
    
    }
    return offset;
}

size_t AST_tree::nasm_generate_return(size_t index, char* line){
    size_t offset = 0;
    Tree_Node curr = get_node(index);
    
    offset+=nasm_generate_expression(curr.left, line);
    size_t off = 0;
    
    sprintf(line+offset,    "\n"
                            "mov rsp, rbp\n"
                            "pop rbp\n"
                            "%n", &off);
    offset += off;

    sprintf(line+offset, "ret\n\n%n",&off);
    return offset+off;
}

size_t AST_tree::nasm_generate_body(size_t index, char* line){

    size_t offset = 0;
    while(index != 0){
        Tree_Node curr = get_node(index);
        offset+=nasm_generate_statement_code(curr.left, line+offset);
        index = curr.right;
    }

    return offset;
}

size_t AST_tree::nasm_generate_loop(size_t index, char* line){
    Tree_Node curr = get_node(index);
    size_t curr_lop = this->curr_label;
    this->curr_label++;
    size_t offset = 0;
    size_t off =0;

    sprintf(line+offset, "loop_LC%lld:\n%n", curr_lop, &off);
    offset+=off;
    offset+=nasm_generate_expression(curr.left, line+offset);

    sprintf(line+offset, "cmp rax, 0\n" 
                         "je LC_end_loop%lld\n%n", curr_lop, &off);
    offset+=off;

    offset+=nasm_generate_body(index, line+offset);

    sprintf(line+offset, "JMP loop_LC%lld\n%n", curr_lop, &off);
    offset+=off;

    sprintf(line+offset, "LC_end_loop%lld:\n%n", curr_lop, &off);
    offset+=off;



    return offset;
}   


size_t AST_tree::nasm_generate_condition(size_t index, char* line){
     Tree_Node curr = get_node(index);
     size_t curr_cond = this->curr_label;
     this->curr_label++;

     size_t offset = 0;
    size_t off = 0;
     Tree_Node right = get_node(curr.right);

    offset+=nasm_generate_expression(curr.left, line+offset);
    
    sprintf(line+offset,    "cmp rax, 0\n"
                            "je LC_condition%lld\n%n", curr_cond, &off);
    offset+=off;

    offset+=nasm_generate_body(right.left, line+offset);
    if(right.right != 0){
        sprintf(line+offset, "jmp LC_end_cond%lld\n%n", curr_cond, &off);
        offset+=off;
    }

    sprintf(line+offset, "LC_condition%lld:\n%n", curr_cond, &off);
    offset+=off;
    offset+=nasm_generate_body(right.right, line+offset);
    
    
    sprintf(line+offset, "LC_end_cond%lld:\n%n", curr_cond, &off);
    offset+=off;


    return offset;
}



size_t AST_tree::nasm_generate_expression(size_t index, char* line){
    size_t off = 0;
    Tree_Node curr = get_node(index);
     switch(curr.type){
        case NUMBER:{
            sprintf(line,  "\n"
                           "mov rax, %lld\n"
                           "\n%n",curr.u.line, &off);
            break;
        }
        case BINARY_OP:{
            size_t offset = 0;
            off+=nasm_generate_expression(curr.right, line+off);
            sprintf(line + off,   "push rax\n%n", &offset);
            off+=offset;

            off+=nasm_generate_expression(curr.left, line+off);
            
            sprintf(line + off,   "pop rbx\n%n", &offset);
            off+=offset;

            off+=nasm_print_binary_operator(index, line+off);
            break;
        }

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

         case VARIABLE:{
             size_t var = this->scope->var_offset(curr.u.line);
             sprintf(line, "mov rax, [rbp + %d]\n%n", var, &off);
             break;
         }

         case READ:{
             sprintf(line, "call fixed_scanf\n%n", &off);
             break;
         }

          case STANDART_FUNCTION:{
              off+=nasm_generate_standart_function(index, line);
              return off;
              break;
          }

         case FUNCTION_CALL:{
             off+=nasm_generate_function_call(index, line);
             break;
         }
    }

    return off;
}



size_t AST_tree::nasm_generate_standart_function(size_t index, char* line){
    Tree_Node curr = get_node(index);
    size_t offset = 0;
    switch((Node_type)curr.u.value){
        case WRITE:{
            offset+= nasm_generate_write(curr.left, line);
            break;
        }
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
        }
    }
    return offset;
}


size_t AST_tree::nasm_generate_sqrt(size_t index, char* line){
    size_t offset = 0;
    size_t off = 0;
    size_t argc = nasm_get_func_argc(index);
    if(argc != 1){
        printf("Wrong count of sqrt arguments");
        fflush(stdout);
        assert(0);
    }
    offset+= nasm_generate_expression(get_node(index).left, line);
    sprintf(line+offset,    "cvtsi2sd xmm0, rax\n"
                            

                            "mov r9, %lld\n"
                            "cvtsi2sd xmm1, r9\n"

                            "divsd xmm0, xmm1\n"

                            "sqrtpd xmm0, xmm0\n"
                            "mulsd xmm0, xmm1\n"
                            // "mul rbx\n"
                            "cvttsd2si rax, xmm0\n"
                            "%n", maximum_of_fractional_part, &off);
    offset+=off;
    return offset;
}


size_t AST_tree::nasm_generate_write(size_t index, char* line){
    size_t count = 0;
    size_t offset = 0;
    size_t off = 0;

    while(index != 0){
        Tree_Node curr = get_node(index);
        offset+=nasm_generate_expression(curr.left, line+offset);
        
        sprintf(line + offset,  "\nmov rdi, rax\n"
                                "call fixed_printf\n"
                                "\n%n", &off);
        
        
        offset+=off;
        index = curr.right;

    }

    return offset;
}


size_t AST_tree::nasm_generate_func_arguments(size_t index, char* line){
    
    size_t offset = 0;
    int i = 0;
    while(index != 0){
        Tree_Node curr = get_node(index);

        offset+=nasm_generate_expression(curr.left, line+offset);
        size_t off = 0;

        sprintf(line+offset, "push rax\n%n", &off);
        offset+=off;
        index = curr.right;
        i++;
    }
    sprintf(line+offset, "\n");
    offset++;
    return offset;
}


size_t AST_tree::nasm_generate_function_call(size_t index, char* line){
    size_t off = 0;

    Tree_Node curr = get_node(index); 
    size_t offset = 0;


    offset+= nasm_generate_func_arguments(curr.left, line+offset);


    sprintf(line+offset, "call %n", &off);
    offset+=off;

    offset+=nasm_generate_label(curr, line+offset);

    size_t index2 = curr.left;

    int count = 0;
    while(index2 != 0){
        Tree_Node curr = get_node(index2);

        count++;

        index2 = curr.right;
    }

    sprintf(line+offset, "\nadd rsp, %d\n%n", count*8, &off);
    offset+=off;


    return offset;

}


size_t AST_tree::nasm_print_binary_operator(size_t index, char* line){
    size_t off = 0;
    Tree_Node curr = get_node(index);

    switch((Math_operators)curr.u.value){
        case PLUS:{
            sprintf(line,"add rax, rbx\n%n", &off);
            return off;
        }
        case MINUS:{
            sprintf(line,"sub rax, rbx\n%n", &off);
            return off;
        }
        case MUL:{
            sprintf(line,   "mov r9, %lld\n\n"
                            "xor rdx, rdx\n"
                            "imul rbx\n"
                            "idiv r9\n"

                            "%n",maximum_of_fractional_part , &off);
            return off;
        }
        case DIV:{
            sprintf(line,   "mov r9, %lld\n"
                            "xor rdx, rdx\n"
                            "imul r9\n"
                            "idiv rbx\n"

                            "%n",maximum_of_fractional_part,  &off);
            return off;
        }
        
    }
    printf("Error. Unknown node %d", curr.type);
    fflush(stdout);
    assert(0);
}




size_t AST_tree::nasm_print_logic(size_t index, char* line){
    size_t off = 0;
    Tree_Node curr = get_node(index);

    switch((Math_operators)curr.u.value){
        case AND:{
            sprintf(line,   "cmp rax, 0\n"
                            "je LCfalse%lld\n"
                            "\n"
                            "\n"
                            "cmp rbx, 0\n"
                            "je LCfalse%lld\n"
                            "\n"
                            "\n"
                            "mov rax, %lld\n"


                            "jmp LCend%lld\n"
                            "LCfalse%lld:\n"
                            "mov rax, 0\n"

                            "LCend%lld:\n"
                            "%n",this->curr_label, this->curr_label, maximum_of_fractional_part + 1, this->curr_label, this->curr_label, this->curr_label,  &off);
            return off;
        }
        case OR:{
            sprintf(line,   "cmp rax, 0\n"
                            "jne LCtrue%lld\n"
                            "\n"
                            "\n"
                            "cmp rbx, 0\n"
                            "jne LCtrue%lld\n"
                            "\n"
                            "\n"
                            "xor rax, rax\n"


                            "jmp LCend%lld\n"
                            "LCtrue%lld:\n"
                            "mov rax, %lld\n"

                            "LCend%lld:\n"
                            "%n",this->curr_label, this->curr_label, this->curr_label, this->curr_label, maximum_of_fractional_part + 1, this->curr_label,  &off);
            return off;
        }
    }
    
    

    printf("Error. Unknown node %d", curr.type);
    fflush(stdout);
    assert(0);
}





size_t AST_tree::nasm_print_logical_operator(size_t index, char* line){
    size_t off = 0;
    Tree_Node curr = get_node(index);

    switch((Math_operators)curr.u.value){
        case EQUAL:{
            sprintf(line,   "cmp rax, rbx\n"
                            "je LCtrue%lld\n"
                            "\n"
                            "mov rax, 0\n"
                            "jmp LCend%lld\n"
                            "LCtrue%lld:\n"
                            "mov rax, %lld\n"
                            "LCend%lld:\n"
                            "%n",this->curr_label, this->curr_label, this->curr_label, maximum_of_fractional_part + 1, this->curr_label,  &off);
            return off;
        }
        case NOT_EQUAL:{
            sprintf(line,   "cmp rax, rbx\n"
                            "jne LCtrue%lld\n"
                            "\n"
                            "mov rax, 0\n"
                            "jmp LCend%lld\n"
                            "LCtrue%lld:\n"
                            "mov rax, %lld\n"
                            "LCend%lld:\n"
                            "%n",this->curr_label, this->curr_label, this->curr_label, maximum_of_fractional_part + 1, this->curr_label,  &off);
            return off;
        }
        case BELOW:{
            sprintf(line,   "cmp rax, rbx\n"
                            "jl LCtrue%lld\n"
                            "\n"
                            "mov rax, 0\n"
                            "jmp LCend%lld\n"
                            "LCtrue%lld:\n"
                            "mov rax, %lld\n"
                            "LCend%lld:\n"
                            "%n",this->curr_label, this->curr_label, this->curr_label, maximum_of_fractional_part + 1, this->curr_label,  &off);
            return off;
        }
        case BELOW_EQUAL:{
            sprintf(line,   "cmp rax, rbx\n"
                            "jle LCtrue%lld\n"
                            "\n"
                            "mov rax, 0\n"
                            "jmp LCend%lld\n"
                            "LCtrue%lld:\n"
                            "mov rax, %lld\n"
                            "LCend%lld:\n"
                            "%n",this->curr_label, this->curr_label, this->curr_label, maximum_of_fractional_part + 1, this->curr_label,  &off);
            return off;
        }
        case ABOVE:{
            sprintf(line,   "cmp rax, rbx\n"
                            "jg LCtrue%lld\n"
                            "\n"
                            "mov rax, 0\n"
                            "jmp LCend%lld\n"
                            "LCtrue%lld:\n"
                            "mov rax, %lld\n"
                            "LCend%lld:\n"
                            "%n",this->curr_label, this->curr_label, this->curr_label, maximum_of_fractional_part + 1, this->curr_label,  &off);
            return off;
        }
        case ABOVE_EQUAL:{
            sprintf(line,   "cmp rax, rbx\n"
                            "jge LCtrue%lld\n"
                            "\n"
                            "mov rax, 0\n"
                            "jmp LCend%lld\n"
                            "LCtrue%lld:\n"
                            "mov rax, %lld\n"
                            "LCend%lld:\n"
                            "%n",this->curr_label, this->curr_label, this->curr_label, maximum_of_fractional_part + 1, this->curr_label,  &off);
            return off;
        }
    }
    
    

    printf("Error. Unknown node %d", curr.type);
    fflush(stdout);
    assert(0);
}