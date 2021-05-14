#pragma once
#include <stdio.h>
#include <cassert>
#include <stdlib.h>
#include "../list/list.hpp"
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>


struct Name_table{
    
    List<const char*>* list_arguments;
    List<const char*>* list_vars;   

    void constructor();
    void destructor();

    bool add_new_var(const char* line);
    bool add_new_argument(const char* line);

    int get_offset(const char* line);

    private:
    
    bool find(const char* line);
     
};



struct Name_pages_list{
    List<Name_table*>* pages;

    void constructor();
    void destructor();

    void new_scope();
    void leave_scope();


    void add_var(const char* line);
    void add_arg(const char* line);


    int var_offset(const char* line);

    private:

    Name_table* last_table();

};

Name_pages_list* New_pages_list();
void Delete_pages_list(Name_pages_list* pages);
