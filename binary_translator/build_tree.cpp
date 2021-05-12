#include <stdio.h>
#include "tree.h"
#include <cassert>
#include <stdlib.h>
#include "../list/list.hpp"
#include "../stack/stack.hpp"
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>


AST_tree::Tree_Node AST_tree::get_node(size_t index){
    return this->list->get_value_by_index(index);
}
bool AST_tree::is_type(Node_type type){
    if(list->get_value_by_index(this->current).type != type){
        return false;
    }else{
        return true;
    }
}

bool AST_tree::is_value(long long val){
    if(list->get_value_by_index(this->current).u.value != val){
        return false;
    }else{
        return true;
    }
}

Node_type AST_tree::get_type(){
    return list->get_value_by_index(this->current).type;
}

long long AST_tree::get_value(){
    return list->get_value_by_index(this->current).u.value;
}

void AST_tree::require(size_t line, Node_type type){
    if(!is_type(type)){
        printf("Line:: %ld  :what i want: %d    what i have:%d so get assert((\n",line,  type, list->get_value_by_index(this->current).type);
        fflush(stdout);
        abort();
    }
}

void AST_tree::left_connection(size_t son, size_t parent){
    Tree_Node tmp = this->list->get_value_by_index(parent);
    tmp.left = son;
    this->list->replace_value_by_index(parent, tmp);
}

void AST_tree::right_connection(size_t son, size_t parent){
    Tree_Node tmp = this->list->get_value_by_index(parent);
    tmp.right = son;
    this->list->replace_value_by_index(parent, tmp);
}

void AST_tree::change_type(size_t index, Node_type type){
    Tree_Node tmp = this->list->get_value_by_index(index);
    tmp.type = type;
    this->list->replace_value_by_index(index, tmp);    
}


void AST_tree::change_value(size_t index, long long val){
    Tree_Node tmp = this->list->get_value_by_index(index);
    tmp.u.value = val;
    this->list->replace_value_by_index(index, tmp);    
}

    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////

    size_t AST_tree::Get_grammar(){
        this->current = 1;
        size_t res = Get_programm();
        require(__LINE__, END);
        return res; 
    }

    size_t AST_tree::Get_programm(){
        size_t prev = Get_func_definition();

        Tree_Node tmp = {};
        tmp.type = LINKER;
        tmp.left = prev;
        this->list->push_back(tmp);

        size_t curr = this->list->size;
        size_t res = curr;
        while(this->list->get_value_by_index(this->current).type == DECLARATOR){

            Tree_Node link = {};
            link.type = LINKER;
            size_t tm = Get_func_definition();
            link.left = tm;
            this->list->push_back(link);

            link.type = LINKER;
            link.left = prev;
            link.right = this->list->size;

            this->list->replace_value_by_index(curr, link);
            curr = this->list->size;
            prev = tm; 
        }
        return res;
    }

    size_t AST_tree::Get_func_definition(){
        require(__LINE__, DECLARATOR);
        this->current++;
        require(__LINE__, VARIABLE);

        size_t curr = this->current;
        this->current++;

        size_t left = Get_func_variables();

        



        size_t right = Get_operations();
         left_connection(left , curr);
        right_connection(right, curr);
        change_type(curr, FUNCTION_DECLARATOR);

        // Tree_Node curr_node = get_node(curr);

        // size_t argc = get_func_argc(curr_node.left);
        // printf("%s  %ld\n", curr_node.u.line, argc);

        // size_t len = strlen(curr_node.u.line);
        // sprintf(curr_node.u.line+len,"%ld", argc);
        // printf("%s  %ld\n", curr_node.u.line, argc);


        return curr;
    }

    size_t AST_tree::Get_func_variables(){
        require(__LINE__, BRACKET_OP);
        this->current++;

        if(is_type(BRACKET_CL)){
            this->current++;
            return 0;
        }

        size_t prev = Get_variable();
        Tree_Node tmp = {};
        tmp.type = LINKER;
        tmp.left = prev;
        this->list->push_back(tmp);

        size_t curr = this->list->size;
        size_t res = curr;
        
        while(get_node(this->current + 1).type == VARIABLE){
            require(__LINE__, COMMA);
            this->current++;
            
            Tree_Node link = {};
            link.type = LINKER;
            size_t tm = Get_variable();
            link.left = tm;
            this->list->push_back(link);

            link.type = LINKER;
            left_connection(prev, curr);
            right_connection(this->list->size, curr);
            curr = this->list->size;
            prev = tm; 
        }
        require(__LINE__, BRACKET_CL);
        this->current++;
        return res;

    }

    size_t AST_tree::Get_variable(){
        this->current++;
        return this->current-1;
    }

    size_t AST_tree::Get_variable_declaration(){
        require(__LINE__, DECLARATOR);
        this->current++;
        require(__LINE__, VARIABLE);
        Tree_Node res = {};
        res.type = VARIABLE_DECLARATOR;
        res.u.line = get_node(this->current).u.line;
        this->list->replace_value_by_index(this->current, res);

        this->current++;
        size_t result = this->current-1;
        Node_type type = get_type();

        if(type == ASSIGNMENT){
        this->current++;

            left_connection(Get_expression(), result);
        }

        return result;
    }

    size_t AST_tree::Get_operations(){
        require(__LINE__, SEPARATOR_OP);
        this->current++;
        size_t res = 0;
        size_t curr = 0;
        if(   is_type(RETURN)            || is_type(VARIABLE)  || is_type(BRACKET_OP) ||
              is_type(STANDART_FUNCTION) || is_type(CONDITION) || is_type(LOOP)       ||
              is_type(NUMBER)            || is_type(DECLARATOR)|| is_type(UNARY)      ||
              is_type(WINDOW)            || is_type(DRAW)){
            res = Get_link_statement();     

        }

        size_t prev = res;

        while(is_type(RETURN)            || is_type(VARIABLE)  || is_type(BRACKET_OP) ||
              is_type(STANDART_FUNCTION) || is_type(CONDITION) || is_type(LOOP)       ||
              is_type(NUMBER)            || is_type(DECLARATOR)|| is_type(UNARY)      ||
              is_type(WINDOW)            || is_type(DRAW)){
            
            curr = Get_link_statement();
            Tree_Node tmp = {};
            tmp = get_node(prev);
            tmp.right = curr;

            this->list->replace_value_by_index(prev, tmp);
            prev = curr;

        }
        
        require(__LINE__, SEPARATOR_CL);
        
        this->current++;
        return res;
    }

    size_t AST_tree::parse_variable_statement(){
        size_t res = 0;
        this->current++;
        Node_type type2 = get_type();
        this->current--;
        switch(type2){
            case ASSIGNMENT:{
                res = Get_assignment();
                break;
            }
            case BRACKET_OP:{
                res = Get_function_call();
                break;
            }
            case UNARY_OP:{
                res = Get_unary_op();
                break;
            }
            default:{
                res = Get_expression();
            }
        }
        return res;
    }

    size_t AST_tree::Get_link_statement(){
        size_t res = 0;
        size_t curr = 0;
        Node_type type = get_type();
        bool end_flag = true;
        // printf("curr_type %d \n", type);
        switch(type){
            case RETURN:{
                curr = Get_return();
                break;
            }
            case LOOP:{
                curr = Get_loop();
                end_flag = false;
                break;
            }
            case CONDITION:{
                curr = Get_condition();
                end_flag = false;
                break;
            }
            case VARIABLE:{
                curr = parse_variable_statement();
                break;
            }
            case STANDART_FUNCTION:{
                curr = Get_standart_function();
                break;
            }

            case DECLARATOR:{
                curr = Get_variable_declaration();
                break;
            }
            case WINDOW:{
                curr = Get_window();
                break;
            }
            case DRAW:{
                curr = Get_draw();
                break;
            }
            default:{
                curr = Get_expression();
            }
        }
        Tree_Node tmp = {};
        tmp.type = LINKER;
        tmp.left = curr;

        this->list->push_back(tmp);
        res = this->list->size;
        if(end_flag){
            require(__LINE__, END_LINE);
            this->current++;   

        }
        return res;
    }

    size_t AST_tree::Get_window(){
        this->current++;
        return this->current-1;
    }

    size_t AST_tree::Get_draw(){
        this->current++;
        return this->current-1;
    }


    size_t AST_tree::Get_loop(){
        require(__LINE__,LOOP);
        size_t res = this->current;
        this->current++;

        require(__LINE__, BRACKET_OP);
        this->current++;
        
        size_t left = Get_expression();
        require(__LINE__, BRACKET_CL);
        this->current++;

        require(__LINE__, SEPARATOR_OP);
        size_t right = Get_operations();
        
        left_connection(left, res);
        right_connection(right, res);

        return res;

    }

    size_t AST_tree::Get_condition(){

        require(__LINE__,CONDITION);
        size_t res = this->current;
        this->current++;

        require(__LINE__, BRACKET_OP);
        this->current++;

        size_t left = Get_expression();
        require(__LINE__, BRACKET_CL);
        this->current++;

        size_t right_left = Get_operations();
        size_t right_right = 0;
        if(is_type(ELSE)){
            this->current++;
            right_right = Get_operations();
        }

        Tree_Node tmp = {};
        tmp.type = LINKER;
        tmp.left = right_left;
        tmp.right = right_right;

        this->list->push_back(tmp);
        size_t right = this->list->size;

        left_connection(left, res);
        right_connection(right, res);

        return res;

    }

    size_t AST_tree::copy_with_str(size_t index){
        Tree_Node tmp = get_node(index);

        char* line = tmp.u.line;
        size_t size = strlen(line);


        char* new_line = (char*)calloc(size+1, sizeof(char));
        memcpy(new_line, line, size);

        tmp.u.line = new_line;
        this->list->push_back(tmp);
        return this->list->size;
    }

    size_t AST_tree::Get_unary_op(){
        size_t left = this->current;
        this->current++;

        size_t res = this->current;
        Math_operators type = (Math_operators)get_value();

        this->current++;
        size_t right_right = Get_expression();
        size_t right_left = copy_with_str(left);

        change_type(res, ASSIGNMENT);
        change_value(res, 0);

        // printf("%d ---\n", type);

        Tree_Node tmp = {};
        tmp.type = BINARY_OP;
        tmp.u.value = type;
        this->list->push_back(tmp);

        size_t right = this->list->size;

        left_connection(left, res);
        right_connection(right, res);
        left_connection(right_left, right);
        right_connection(right_right, right);
        return res;

    }



    size_t AST_tree::Get_assignment(){
        size_t left = this->current;
        require(__LINE__, VARIABLE);
        this->current++;
        require(__LINE__, ASSIGNMENT);
        size_t res = this->current;
        this->current++;
        size_t right = Get_expression();
        left_connection(left, res);
        right_connection(right, res);

        return res;
    }


    size_t AST_tree::Get_standart_function(){
        require(__LINE__, STANDART_FUNCTION);
        size_t res = this->current;
        this->current++;
        left_connection(Get_expression_args(), res);

        return res;

    }


    size_t AST_tree::Get_return(){
        require(__LINE__, RETURN);
        size_t res = this->current;
        this->current++;

        left_connection(Get_expression(), res);

        return res;
    }

    size_t AST_tree::Get_expression(){
        size_t prev = Get_logic_expression();
        while(is_type(LOGIC) ){
            // printf("okkkkkkkkkk\n");

            size_t op = this->current;
            this->current++;
            size_t curr =  Get_logic_expression();

            left_connection(prev, op);
            right_connection(curr, op);

            prev = op;
        }
        return prev;
    }

    size_t AST_tree::Get_logic_expression(){
        size_t prev = Get_part_of_expression();
        while(is_type(LOGIC_OP) ){
            // printf("okkkkkkkkkk\n");

            size_t op = this->current;
            this->current++;
            size_t curr =  Get_part_of_expression();

            left_connection(prev, op);
            right_connection(curr, op);

            prev = op;
        }
        return prev;
    }

    size_t AST_tree::Get_part_of_expression(){
        size_t prev = Get_term();
        while(is_type(BINARY_OP) && (is_value(PLUS) || is_value(MINUS))){
            // printf("okkkkkkkkkk\n");
            size_t op = this->current;
            this->current++;
            size_t curr =  Get_term();
            left_connection(prev, op);
            right_connection(curr, op);
            prev = op;
        }
        return prev;
    }



    size_t AST_tree::Get_term(){
        size_t prev = Get_power();
        while(is_type(BINARY_OP) && (is_value(MUL) || is_value(DIV))){
            // printf("okkkkkkkkkk\n");
            size_t op = this->current;
            this->current++;
            size_t curr =  Get_power();
            
            left_connection(prev, op);
            right_connection(curr, op);

            prev = op;
        }
        return prev;
    }

    size_t AST_tree::Get_power(){
        size_t prev = Get_unar();
        while(is_type(BINARY_OP) && is_value(POW)){
            // printf("okkkkkkkkkk\n");
            size_t op = this->current;
            this->current++;
            size_t curr =  Get_unar();
            
            left_connection(prev, op);
            right_connection(curr, op);

            prev = op;
        }
        return prev;
    }




    size_t AST_tree::Get_unar(){

        bool flag = false;
        size_t res = 0;
        size_t count = 0; 
        while(is_type(UNARY)){
            if(!is_value(MINUS)){
                printf("WRONG UNARY %lld", get_value());
            }
            flag = true;
            count++;
            this->current++;
        }
        if(count %2 == 0){
            return Get_primary_expression();
        }else{
            Tree_Node tmp = {};
            tmp.u.value = -maximum_of_fractional_part;
            tmp.type = NUMBER;

            this->list->push_back(tmp);
            size_t left = this->list->size;

            tmp.u.value = MUL;
            tmp.type = BINARY_OP;

            this->list->push_back(tmp);
            size_t res = this->list->size;

            left_connection(left, res);
            right_connection(Get_primary_expression(), res);
            return res;
        }
    }


    size_t AST_tree::Get_primary_expression(){
        Node_type type = get_type();
        switch(type){
            case BRACKET_OP:{
                this->current++;
                size_t res = Get_expression();
                require(__LINE__, BRACKET_CL);
                this->current++;
                return res;
            }

            case NUMBER:{
                this->current++;
                return this->current-1;
            }

            case VARIABLE:{
                this->current++;
                if(is_type(BRACKET_OP)){
                    this->current--;
                    return Get_function_call();
                }else{
                    return this->current-1;
                }
            }

            case STANDART_FUNCTION:{
                return Get_standart_function();
            }

            case READ:{
                return Get_read();
            }

        }
        printf("ERROR: wrong type:%d\n", type);
        fflush(stdout);
        assert(0);
    }

    size_t AST_tree::Get_read(){
        require(__LINE__,READ);
        this->current++;
        return this->current-1;
    }

    size_t AST_tree::Get_function_call(){
        size_t res = this->current;
        this->current++;
        change_type(res, FUNCTION_CALL);
        left_connection(Get_expression_args(), res);

        // Tree_Node curr = get_node(res);

        // size_t argc = get_func_argc(curr.left);
        // printf("%s  %ld\n", curr.u.line, argc);

        // size_t len = strlen(curr.u.line);
        // sprintf(curr.u.line+len,"%ld", argc);
        // printf("%s  %ld\n", curr.u.line, argc);

        return res;
    }

    size_t AST_tree::Get_expression_args(){
        require(__LINE__, BRACKET_OP);
        this->current++;

        if(is_type(BRACKET_CL)){
            this->current++;
            return 0;
        }

        size_t prev = Get_expression();

        Tree_Node tmp = {};
        tmp.type = LINKER;
        tmp.left = prev;
        this->list->push_back(tmp);

        size_t curr = this->list->size;
        size_t res = curr;
        this->current++;
        bool flag = is_type(VARIABLE) || is_type(NUMBER) || is_type(READ) || is_type(STANDART_FUNCTION) || is_type(UNARY); 
        this->current--;
        while(flag){
            // printf("-%d ", flag);

            require(__LINE__, COMMA);
            this->current++;
            
            Tree_Node link = {};
            link.type = LINKER;
            size_t tm = Get_expression();
            link.left = tm;
            this->list->push_back(link);

            link.type = LINKER;
            link.left = prev;
            link.right = this->list->size;

            this->list->replace_value_by_index(curr, link);
            curr = this->list->size;
            prev = tm;
            this->current++; 
            flag = is_type(VARIABLE) || is_type(NUMBER) || is_type(READ) || is_type(STANDART_FUNCTION)  || is_type(UNARY); 
            this->current--;
        }
        require(__LINE__, BRACKET_CL);
        this->current++;
        return res;
    }



    size_t AST_tree::nasm_get_func_argc(size_t index){
        size_t count = 0;
        while(index != 0){
            count++;
            index = get_node(index).right;
        }
        return count;
    }
