#include <stdio.h>
#include "tree.h"

void AST_tree::create_assemble_file(const char* file){
    this->curr_condition = 0;
    this->curr_loop = 0;
    FILE* fp = fopen(file, "w");
    char* buffer = (char*)calloc(max_buffer_size+1, sizeof(char));
    size_t offset = 0;
    sprintf(buffer,"JMP main0\n\n\n%n",&offset);

    size_t size = generate_code(buffer+offset);

    fwrite(buffer,size+offset, sizeof(char), fp);
    fclose(fp);
    free(buffer);
}

size_t AST_tree::generate_code(char* line){
    this->scope = New_pages_list(); 

    size_t curr = this->root;
    size_t offset = 0;
    while(curr != 0){
        Tree_Node tmp = get_node(curr);
        offset+= generate_function_code(tmp.left, line + offset);
        curr = tmp.right;
    };

    Delete_pages_list(this->scope);
    this->scope = NULL;
    return offset;
}

size_t AST_tree::generate_default_return(char* line){
    size_t offset = 0;
    sprintf(line,"/default return\n"
                 "PUSH 0\n"
                 "RET\n\n%n",&offset);
    return offset;
}

size_t AST_tree::generate_body_code(size_t index, char* line){
    Tree_Node func = get_node(index);
    size_t offset = 0;
    size_t curr = index;
    while(curr != 0){
        // printf("%ld\n", curr);
        Tree_Node tmp = get_node(curr);
        offset+= generate_statement_code(tmp.left,line + offset);
        curr = tmp.right;
    }
    return offset;
}


void AST_tree::get_function_arguments(size_t index){
    size_t curr = index;
    while(curr != 0){
        Tree_Node Link = get_node(curr);
        Tree_Node tmp = get_node(Link.left);
        this->scope->add_var(tmp.u.line);
        curr = Link.right;
    }
}

size_t AST_tree::generate_label(AST_tree::Tree_Node node, char* line){
        size_t argc = get_func_argc(node.left);
        size_t offset = 0;
        sprintf(line,"%s%ld%n", node.u.line, argc, &offset);

        return offset;
}


size_t AST_tree::generate_function_code(size_t index, char* line){
    this->scope->new_scope();
    // printf("fcode ");

    Tree_Node func = get_node(index);
    size_t offset = 0;

    offset+=generate_label(func, line+offset);

    size_t off = 0;
    sprintf(line+offset,":\n\n%n", &off);
    offset+= off;


    get_function_arguments(func.left);
    offset+=generate_body_code(func.right, line+offset);
    offset+=generate_default_return(line+offset);

    this->scope->leave_scope();
    return offset;
}

size_t AST_tree::generate_statement_code(size_t index, char* line){
    Tree_Node node = get_node(index);
    size_t offset = 0;
    switch(node.type){
        case ASSIGNMENT:{
            return generate_assignment(index, line);
        }
        case RETURN:{
            return generate_return(index, line);
        }
        case VARIABLE_DECLARATOR:{
            return generate_variable_declaration(index, line);
        }
        case CONDITION:{
            return generate_condition(index, line);
        }
        case LOOP:{
            return generate_loop(index, line);
        }
        case WINDOW:{
            return generate_window(index, line);
        }
        case DRAW:{
            return generate_draw(index, line);
        }
        default:{
            return generate_expression(index, line); 
        }
    }
}   

size_t AST_tree::generate_window(size_t index, char* line){
    size_t off = 0;
    sprintf(line, "WIN 512 512\n%n",&off);
    return off;
}

size_t AST_tree::generate_draw(size_t index, char* line){
    size_t off = 0;
    sprintf(line, "DRAW\n%n",&off);
    return off;
}

size_t AST_tree::generate_loop(size_t index, char* line){
    Tree_Node curr = get_node(index);
    size_t curr_lop = this->curr_loop;
    this->curr_loop++;
    size_t offset = 0;
    size_t off =0;
    sprintf(line+offset, "///////////LOOP//////////\n%n", &off);
    offset+=off;
    
    
    sprintf(line+offset, "begin_loop*%ld:\n%n", curr_lop, &off);
    offset+=off;
    offset+=generate_expression(curr.left, line+offset);

    sprintf(line+offset, "PUSH 0\n" 
                         "JE end_loop*%ld\n%n", curr_lop, &off);
    offset+=off;

    offset+=generate_body(index, line+offset);

    sprintf(line+offset, "JMP begin_loop*%ld\n%n", curr_lop, &off);
    offset+=off;

    sprintf(line+offset, "end_loop*%ld:\n%n", curr_lop, &off);
    offset+=off;

    sprintf(line+offset, "/////////////////////////\n%n", &off);
    offset+=off;

    return offset;
}   


size_t AST_tree::generate_condition(size_t index, char* line){
    Tree_Node curr = get_node(index);
    size_t curr_cond = this->curr_condition;
    this->curr_condition++;
    size_t offset = 0;
    Tree_Node right = get_node(curr.right);
    sprintf(line,"//////////////CONDITION//////\n%n", &offset);
    offset+=generate_expression(curr.left, line+offset);
    size_t off = 0;
    sprintf(line+offset,"PUSH 0\n"
                 "JE condition*%ld\n%n", curr_cond, &off);
    offset+=off;

    offset+=generate_body(right.left, line+offset);
    sprintf(line+offset, "JMP end_cond*%ld\n%n", curr_cond, &off);
    offset+=off;

    sprintf(line+offset, "condition*%ld:\n%n", curr_cond, &off);
    offset+=off;
    offset+=generate_body(right.right, line+offset);
    
    
    sprintf(line+offset, "end_cond*%ld:\n%n", curr_cond, &off);
    offset+=off;
    sprintf(line+offset,"////////////////////\n\n%n", &off);
    offset+=off;

    return offset;
}

size_t AST_tree::generate_body(size_t index, char* line){
    this->scope->new_layer();
    size_t offset = 0;
    while(index != 0){
        Tree_Node curr = get_node(index);
        offset+=generate_statement_code(curr.left, line+offset);
        index = curr.right;
    }
    this->scope->leave_layer();
    return offset;
}


size_t AST_tree::generate_variable_declaration(size_t index, char* line){
    Tree_Node curr = get_node(index);
    this->scope->add_var(curr.u.line);
    size_t offset = 0;
    if(curr.left != 0){
        offset+=generate_expression(curr.left, line);
        size_t off = 0;
        sprintf(line+offset, "\nPOP [rcx+%ld]\n%n", this->scope->var_offset(curr.u.line), &off);
        offset+=off;
    }
    return offset;
}



size_t AST_tree::generate_return(size_t index, char* line){
    size_t offset = 0;
    Tree_Node curr = get_node(index);
    offset+=generate_expression(curr.left, line);
    size_t off = 0;
    sprintf(line+offset, "RET\n\n%n",&off);
    return offset+off;
}


size_t AST_tree::generate_assignment(size_t index, char* line){
    Tree_Node node = get_node(index);
    size_t offset = 0;
    size_t off = 0;
    
    sprintf(line + offset, "/assignment\n%n",&off);
    offset+=off;
    offset += generate_expression(node.right, line + offset);
    
    Tree_Node tmp = get_node(node.left);
    size_t var = this->scope->var_offset(tmp.u.line);
    
    sprintf(line + offset, "POP [rcx+%ld]\n%n", var, &off);
    offset+=off;
    sprintf(line + offset,"\n");
    offset++;
    return offset;
}

size_t AST_tree::generate_expression(size_t index, char* line){
    size_t off = 0;
    Tree_Node curr = get_node(index);
    switch(curr.type){
        case NUMBER:{
            sprintf(line, "PUSH %15.15lg\n%n",curr.u.value, &off);
            break;
        }
        case BINARY_OP:{
            off+=generate_expression(curr.left, line+off);
            off+=generate_expression(curr.right, line+off);
            off+=print_binary_operator(index, line+off);
            break;
        }

        case LOGIC_OP:{
            off+=generate_expression(curr.left, line+off);
            off+=generate_expression(curr.right, line+off);
            off+=print_logical_operator(index, line+off);
            break;
        }

        case LOGIC:{
            off+=generate_expression(curr.left, line+off);
            off+=generate_expression(curr.right, line+off);
            off+=print_logic(index, line+off);
            break;
        }

        case VARIABLE:{
            size_t var = this->scope->var_offset(curr.u.line);
            sprintf(line, "PUSH [rcx+%ld]\n%n", var, &off);
            break;
        }

        case READ:{
            sprintf(line, "IN\n%n", &off);
            break;
        }

        case STANDART_FUNCTION:{
            off+=generate_standart_function(index, line);
            break;
        }

        case FUNCTION_CALL:{
            off+=generate_function_call(index, line);
            break;
        }
    }
    // sprintf(line, "PUSH 1\n%n", &off);
    return off;
}

size_t AST_tree::generate_func_arguments(size_t index, char* line){
    size_t delta = this->scope->register_offset();
    
    size_t offset = 0;
    int i = 0;
    while(index != 0){
        Tree_Node curr = get_node(index);

        offset+=generate_expression(curr.left, line+offset);
        size_t off = 0;

        sprintf(line+offset, "POP [rcx+%ld]\n%n", delta+i, &off);
        offset+=off;
        index = curr.right;
        i++;
    }
    sprintf(line+offset, "\n");
    offset++;
    return offset;
}



size_t AST_tree::generate_function_call(size_t index, char* line){
    size_t delta = this->scope->register_offset();
    size_t off = 0;

    Tree_Node curr = get_node(index); 
    size_t offset = 0;
    sprintf(line,"/////////////CALL///////////\n%n",&offset);

    offset+= generate_func_arguments(curr.left, line+offset);


    sprintf(line+offset,"PUSH rcx\n"
                        "PUSH %ld\n"
                        "ADD\n"
                        "POP rcx\n"
                        "CALL %n",  delta, &off);
    offset+=off;

    offset+=generate_label(curr, line+offset);

    sprintf(line+offset,"\n\n"
                        "PUSH rcx\n"
                        "PUSH %ld\n"
                        "SUB\n"
                        "POP rcx\n"
                        "%n", delta, &off);
    offset+=off;
    sprintf(line+offset,"//////////////////////////\n\n%n",&off);
    offset+=off;

    return offset;

}




size_t AST_tree::generate_standart_function(size_t index, char* line){
    Tree_Node curr = get_node(index);
    size_t offset = 0;
    switch((Node_type)curr.u.value){
        case WRITE:{
            offset+= generate_write(curr.left, line);
            break;
        }
        case PUT_PIXEL:{
            offset+=generate_pixel(curr.left, line);
            break;
        }
        case SIN:{
            offset+=generate_sin(curr.left, line);
            break;
        }
        case ABS:{
            offset+=generate_abs(curr.left, line);
            break;
        }
        case COS:{
            offset+=generate_cos(curr.left, line);
            break;
        }
    }
    return offset;
}

size_t AST_tree::generate_sin(size_t index, char* line){
    size_t offset = 0;
    size_t off = 0;
    size_t argc = get_func_argc(index);
    if(argc != 1){
        printf("Wrong count of sin arguments");
        fflush(stdout);
        assert(0);
    }
    offset+= generate_expression(get_node(index).left, line);
    sprintf(line+offset, "SIN\n%n", &off);
    offset+=off;
    return offset;
}   

size_t AST_tree::generate_abs(size_t index, char* line){
    size_t offset = 0;
    size_t off = 0;
    size_t argc = get_func_argc(index);
    if(argc != 1){
        printf("Wrong count of abs arguments");
        fflush(stdout);
        assert(0);
    }
    offset+= generate_expression(get_node(index).left, line);
    sprintf(line+offset, "ABS\n%n", &off);
    offset+=off;
    return offset;
}

size_t AST_tree::generate_cos(size_t index, char* line){
    size_t offset = 0;
    size_t off = 0;
    size_t argc = get_func_argc(index);
    if(argc != 1){
        printf("Wrong count of cos arguments");
        fflush(stdout);
        assert(0);
    }
    offset+= generate_expression(get_node(index).left, line);
    sprintf(line+offset, "COS\n%n", &off);
    offset+=off;
    return offset;
}


size_t AST_tree::generate_pixel(size_t index, char* line){
    size_t offset = 0;
    size_t off = 0;
    size_t argc = get_func_argc(index);
    if(argc != 3){
        printf("Wrong count of Pixel arguments");
        fflush(stdout);
        assert(0);
    }

    Tree_Node curr = get_node(index);
    offset+= generate_expression(curr.left, line+offset);
    sprintf(line+offset, "PUSH 256\n"
                         "ROUND\n"
                         "ADD\n"
                         "ROUND\n"
                         "PUSH 512\n"
                         "MUL\n"
                         "ROUND\n\n%n", &off);
    offset+=off;

    curr = get_node(curr.right);
    

    offset+= generate_expression(curr.left, line+offset);
    sprintf(line+offset, "PUSH 256\n"
                         "ADD\n"
                         "ROUND\n"
                         "ADD\n"
                         "PUSH 262144\n"
                         "ADD\n"
                         "ROUND\n"
                         "POP rex\n\n%n", &off);
    offset+=off;

    curr = get_node(curr.right);
    offset+=generate_expression(curr.left, line+offset);

    sprintf(line+offset, "POP [rex]\n\n%n", &off);
    offset+=off;

    return offset;
}



size_t AST_tree::generate_write(size_t index, char* line){
    size_t count = 0;
    size_t offset = 0;
    size_t off = 0;
    while(index != 0){
        Tree_Node curr = get_node(index);
        offset+=generate_expression(curr.left, line+offset);
        sprintf(line + offset, "OUT\n\n%n", &off);
        offset+=off;
        index = curr.right;

    }
    // sprintf(line+offset,"PUSH %ld\n%n",count, &off);
    // offset+=off;
    return offset;
}



size_t AST_tree::print_binary_operator(size_t index, char* line){
    size_t off = 0;
    Tree_Node curr = get_node(index);

    switch((Math_operators)curr.u.value){
        case PLUS:{
            sprintf(line,"ADD\n%n", &off);
            return off;
        }
        case MINUS:{
            sprintf(line,"SUB\n%n", &off);
            return off;
        }
        case MUL:{
            sprintf(line,"MUL\n%n", &off);
            return off;
        }
        case DIV:{
            sprintf(line,"DIV\n%n", &off);
            return off;
        }
        case POW:{
            sprintf(line,"POW\n%n", &off);
            return off;
        }
    }
    printf("Error. Unknown node %d", curr.type);
    fflush(stdout);
    assert(0);
}


size_t AST_tree::print_logical_operator(size_t index, char* line){
    size_t off = 0;
    Tree_Node curr = get_node(index);

    switch((Math_operators)curr.u.value){
        case EQUAL:{
            sprintf(line,"EQUAL\n%n", &off);
            return off;
        }
        case NOT_EQUAL:{
            sprintf(line,"NEQUAL\n%n", &off);
            return off;
        }
        case BELOW:{
            sprintf(line,"LESS\n%n", &off);
            return off;
        }
        case BELOW_EQUAL:{
            sprintf(line,"LESSE\n%n", &off);
            return off;
        }
        case ABOVE:{
            sprintf(line,"MORE\n%n", &off);
            return off;
        }
        case ABOVE_EQUAL:{
            sprintf(line,"MOREE\n%n", &off);
            return off;
        }
    }
    
    

    printf("Error. Unknown node %d", curr.type);
    fflush(stdout);
    assert(0);
}

size_t AST_tree::print_logic(size_t index, char* line){
    size_t off = 0;
    Tree_Node curr = get_node(index);

    switch((Math_operators)curr.u.value){
        case AND:{
            sprintf(line,"AND\n%n", &off);
            return off;
        }
        case OR:{
            sprintf(line,"OR\n%n", &off);
            return off;
        }
    }
    
    

    printf("Error. Unknown node %d", curr.type);
    fflush(stdout);
    assert(0);
}
