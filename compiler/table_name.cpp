#include <stdio.h>
#include <cassert>
#include <stdlib.h>
#include "../list/list.hpp"
#include "table_name.h"
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>


Name_table* New_table(){
    Name_table* res = (Name_table*)calloc(1, sizeof(Name_table));
    res->constructor();
    return res;
}

void Delete_table(Name_table* table){
    table->destructor();
    free(table);
}

    void Name_table::constructor(){
        this->list_arguments = (List<const char*>*)calloc(1, sizeof(List<const char*>));
        this->list_arguments->constructor();

        this->list_vars = (List<const char*>*)calloc(1, sizeof(List<const char*>));
        this->list_vars->constructor();
        
    }   

    void Name_table::destructor(){
        this->list_arguments->destructor();
        free(this->list_arguments);
        this->list_arguments = NULL;

        this->list_vars->destructor();
        free(this->list_vars);
        this->list_vars = NULL;

    }

    bool Name_table::add_new_argument(const char* line){
        if(find(line)){
            return false;
        }

        size_t tmp = 0;

        this->list_arguments->push_back(line);
        return true;
    }
 
    bool Name_table::add_new_var(const char* line){
        if(find(line)){
            return false;
        }

        size_t tmp = 0;

        this->list_vars->push_back(line);
        return true;
    }

    
    int Name_table::get_offset(const char* line){
        size_t size = this->list_arguments->size;

        for(int i = 1; i <= size; i++){
            const char* curr = this->list_arguments->get_value_by_index(i);
            if(strcmp(curr, line) == 0){
                return (size + 2 - i) * 8;
            }
        }

        size = this->list_vars->size;

        for(int i = 1; i <= size; i++){
            const char* curr = this->list_vars->get_value_by_index(i);
            if(strcmp(curr, line) == 0){
                return (-i) * 8;
            }
        }
        return 0;
    }


    bool Name_table::find(const char* line){
        size_t size = this->list_arguments->size;

        for(int i = 1; i <= size; i++){
            const char* curr = this->list_arguments->get_value_by_index(i);
            if(strcmp(curr, line) == 0){
                return true;
            }
        }

        size = this->list_vars->size;

        for(int i = 1; i <= size; i++){
            const char* curr = this->list_vars->get_value_by_index(i);
            if(strcmp(curr, line) == 0){
                return true;
            }
        }
        return false;
    }
     
     

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

Name_pages_list* New_pages_list(){
    Name_pages_list* res = (Name_pages_list*)calloc(1, sizeof(Name_pages_list));
    res->constructor();
    return res;
}

void Delete_pages_list(Name_pages_list* pages){
    pages->destructor();
    free(pages);
}

    void Name_pages_list::constructor(){
        this->pages = (List<Name_table*>*)calloc(1, sizeof(List<Name_table*>));
        this->pages->constructor();
    }


    void Name_pages_list::new_scope(){
        Name_table* tab = New_table();
        this->pages->push_back(tab);
    }

    void Name_pages_list::leave_scope(){
        Name_table* tmp = this->pages->pop_back();
        Delete_table(tmp);
    }

    void Name_pages_list::add_var(const char* line){
        if(!last_table()->add_new_var(line)){
            printf("Redeclaraton of variable");
            fflush(stdout);
            assert(0);
        }   
    }

    void Name_pages_list::add_arg(const char* line){
        if(!last_table()->add_new_argument(line)){
            printf("Redeclaraton of variable");
            fflush(stdout);
            assert(0);
        }   
    }

    void Name_pages_list::destructor(){
        size_t size = this->pages->size;

        for(int i = 1; i <= size; i++){
            this->pages->get_value_by_index(i)->destructor();
        }

        this->pages->destructor();
        free(this->pages);
        this->pages = NULL;
    }

   
    int Name_pages_list::var_offset(const char* line){
        int result = last_table()->get_offset(line);

        if(!result){
            printf("Unknown identifier :%s \n", line);
            fflush(stdout);
            assert(0);
        }
        return result;

    }

    Name_table* Name_pages_list::last_table(){
        return this->pages->back();
    }
