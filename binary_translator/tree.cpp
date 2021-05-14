#include <stdio.h>
#include "tree.h"
#include <cassert>
#include <stdlib.h>
#include "../list/list.hpp"
#include "../stack/stack.hpp"
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>

void AST_tree::destructor(){
    for(int i = 1; i <= list->count(); i++){
        Tree_Node tmp = list->get_value_by_index(i);
        if(tmp.type == VARIABLE      || tmp.type == FUNCTION_DECLARATOR ||
           tmp.type == FUNCTION_CALL || tmp.type == VARIABLE_DECLARATOR){
            free(tmp.u.line);
        }
    }
    list->destructor();
    free(list);
    this->list = NULL;
    if(this->jit_buffer){
        free(this->jit_buffer);
        this->jit_buffer = NULL;
    }
}

void AST_tree::build_AST(){
    this->root = Get_grammar();
}

void AST_tree::dump_tree(const char* file){
    FILE* fp = fopen("tmp.gv", "w");
    fprintf(fp,"digraph G{\n");
    do_tree_print(this->root, fp);
    fprintf(fp,"}\n");
    fclose(fp);

    char* str = (char*)calloc(23+strlen(file), sizeof(char));
    strcat(str, "dot -Tpdf tmp.gv -o");
    strcat(str, file);
    system(str);
    free(str);
}

void AST_tree::do_tree_print(size_t index, FILE* fp){
    fprintf(fp, "%ld[shape=record label = \"",index);
    Tree_Node tmp = this->list->get_value_by_index(index);
    // printf("%ld  left=%ld  right=%ld\n", index, tmp.left, tmp.right);
    node_print(fp, &tmp);
    fprintf(fp,"| curr =  %ld|{left = %ld| right = %ld}\"]\n", index, tmp.left, tmp.right);

    if(tmp.left != 0){

        fprintf(fp, "%ld->%ld[label = L]\n", index, tmp.left);        
        do_tree_print(tmp.left, fp);
    }
    if(tmp.right != 0){
        fprintf(fp, "%ld->%ld[label = R]\n",index , tmp.right);        
        do_tree_print(tmp.right, fp);
    }
}



void AST_tree::dump_list(const char* file){
    assert(list != NULL);
    assert(list->data != NULL);
    assert(file != NULL);

    FILE* fp = fopen("tmp.gv", "w");
    fprintf(fp,"digraph G{\n");
    fprintf(fp, "   graph[splines =true];\n");

    for(int i = 0; i <= list->capacity; i++){
        fprintf(fp, "_%d[shape=record,label=\" val = ",i);
        node_print(fp, &(list->data[i].value));
        fprintf(fp, " | {<prev> prev = %d | <next> next = %d} | ind = %d\" style = filled fillcolor=\"#F3FDC9\"];\n ", list->data[i].prev, list->data[i].next, i);
        if(list->data[i].prev == -1){
            fprintf(fp, "_%d[style = filled fillcolor=\"#8BF696\"];\n", i);
        }else{
        }
    }

    fprintf(fp, "_0[style = filled fillcolor=\"#A4BEF2\"];\n");

    for(int curr = 0; curr <= list->capacity; curr++){
        if(list->data[curr].prev != -1){
            fprintf(fp, "_%d->_%d\n", curr, list->data[curr].next);
            fprintf(fp, "_%d->_%d\n", curr, list->data[curr].prev);
        }else{
            fprintf(fp, "_%d->_%d\n", curr, list->data[curr].next);
        }
    }

    size_t curr = list->head;

    for(int i = 0; i < list->size; i++){
        fprintf(fp, "%d[shape=record label=\"pos = %d \"]",i, i+1);
        fprintf(fp, "%d->_%d[minlen=30]\n", i, curr);
        if(curr != -1 && curr <= list->capacity){
            curr = list->data[curr].next;
        }
    }

    fprintf(fp,"}\n");
    fclose(fp);
    char* str = (char*)calloc(23+strlen(file), sizeof(char));
    strcat(str, "circo -Tpdf tmp.gv -o");
    strcat(str, file);
    system(str);
    free(str);
}

    void AST_tree::node_print(FILE* fp, Tree_Node* nd){
        if(nd->type == VARIABLE){
            fprintf(fp, "var %s", nd->u.line);
            return;
        }

        if(nd->type == FUNCTION_DECLARATOR){
            fprintf(fp, "function %s", nd->u.line);
            return;
        }

        if(nd->type == VARIABLE_DECLARATOR){
            fprintf(fp, "var declaration %s", nd->u.line);
            return;
        }

        if(nd->type == FUNCTION_CALL){
            fprintf(fp, "call %s", nd->u.line);
            return;
        }

        if(nd->type == LINKER){
            fprintf(fp, "LINK");
            return;
        }

        if(nd->type == NUMBER){
            fprintf(fp, "%lld", nd->u.value);
            return;
        }

        if(nd->type == SEPARATOR_OP){
            fprintf(fp, "SEPARATOR_OP");
            return;
        }

        if(nd->type == SEPARATOR_CL){
            fprintf(fp, "SEPARATOR_CL");
            return;
        }

        for(int i = 0; i < node_types_count; i++){
            if(nd->type == node_types[i].type && nd->u.value == node_types[i].value){
                fprintf(fp, "%s", node_types[i].line);
                return;
            }
        }
    }
