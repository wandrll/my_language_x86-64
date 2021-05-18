#include <stdio.h>
#include "compiler.h"
#include <cassert>
#include <stdlib.h>
#include "../list/list.hpp"
#include "../stack/stack.hpp"
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>


void Compiler::lexical_analysis(const char* file){

    Stack_t<Error> errors = {};
    errors.constructor(10);
    list->constructor();

    size_t fsize = file_size(file);
    char* buffer = (char*)calloc(fsize+1, sizeof(char));

    FILE* fp = fopen(file, "rb");
    if(fp == NULL){
        printf("Fatal error, couldnt open file %s\n", file);
        abort();
    }

    fread(buffer, sizeof(char), fsize, fp);
    fclose(fp);

    char* curr = buffer;
    Tree_Node tmp = {};

    while(*curr != 0){
        while(is_garbage(*curr)){
            curr++;
        }
        if(*curr == 0){
            break;
        }
        bool flag = false;
        for(int i = 0; i < node_types_count; i++){
            if(strncmp(node_types[i].line, curr, node_types[i].str_size) == 0 && !flag){
                tmp.type = node_types[i].type;
                tmp.u.value = node_types[i].value;
                list->push_back(tmp);
                curr += node_types[i].str_size;
                flag = true;
            }
        }

        if(flag && (tmp.type == BINARY_OP)){
            size_t size = this->list->size;
            Tree_Node prev = this->list->get_value_by_index(size-1);
            if(prev.type == BINARY_OP || prev.type == UNARY        || prev.type == LOGIC_OP     ||
               prev.type == ASSIGNMENT|| prev.type == SEPARATOR_OP || prev.type == BRACKET_OP   ||
               prev.type == COMMA){
                tmp.type = UNARY;
                // tmp.u.value = tmp.u.value * 16777216;
                this->list->replace_value_by_index(size, tmp);
            }
        }
        if(!flag){
            if(isalpha(*curr)){
                curr += push_variable(curr);
            }else{
                if(isnumber(curr)){
                    curr += push_value(curr);
                }else{
                    Error tmp_err = {};
                    tmp_err.pos = curr-buffer;
                    tmp_err.symbol = *curr;
                    errors.push(tmp_err);
                    curr++;
                }
            }
        }
    }

    if(errors.size != 0){
        print_errors(&errors);
    }

    free(buffer);
    errors.destructor();
    tmp.type = END;
    list->push_back(tmp);
}    


    size_t Compiler::push_variable(char* curr){
        size_t len = var_len(curr);
        char* line = (char*) calloc(len + 15, sizeof(char));
        memcpy(line, curr, len);
        curr += len;
        Tree_Node tmp = {};
        tmp.type = VARIABLE;
        tmp.u.line = line;
        list->push_back(tmp);
        return len;
    }

    size_t Compiler::push_value(char* curr){
        long long val1 = 0;
        double val2 = 0;
        size_t off = 0;
        size_t offset = 0;
        sscanf(curr, "%lg%n", &val2, &offset);
        off+=offset;
        curr+=offset;

        
        val1 = (long long)(val2 * maximum_of_fractional_part);

        Tree_Node tmp = {};
        tmp.type = NUMBER;
        tmp.u.value = val1;
        list->push_back(tmp);
        return off;
    }

    void Compiler::print_errors(Stack_t<Error>* st){
        size_t size = st->size;
        printf("%ld", size);
        for(int i = 0; i < size; i++){
            Error tmp = {};
            st->pop(&tmp);
            printf("I don't know what is it::position: %ld :: %c\n", tmp.pos, tmp.symbol);
        }
        fflush(stdout);
        abort();
    }
    
    size_t Compiler::file_size(const char* file){
        assert(file != NULL);
        struct stat st = {};
        stat(file, &st);
        return st.st_size;
    }

    bool Compiler::is_garbage(char c){
        if(c == ' ' || c == '\n' || c == '\t'){
            return true;
        }
        return false;
    }

    size_t Compiler::var_len(char* line){
        size_t res = 0;
        while(isalpha(*line) || isdigit(*line) || *line == '_'){
            line++;
            res++;
        }
        return res;
    }

    bool Compiler::isnumber(char* line){
        if(isdigit(*line)){
            return true;
        }

        if((*line =='-' || *line == '+') && isdigit(*(line+1))){
            return true;
        }else{
            return false;
        }

    }
