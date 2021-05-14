#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "list.hpp"
#include "hash_map.hpp"


void hash_map_constructor(Hash_map* ths){
    ths->data = (List_for_map**)calloc(1024, sizeof(List_for_map*));

    ths->load_factor = 0;
    ths->size = 0;
    ths->bucket_count = 1024;

}

static void free_buckets(Hash_map* ths){

    for(int i = 0; i < ths->bucket_count; i++){
        if(ths->data[i]){
            size_t count = ths->data[i]->size;
            for(int j = 0; j < count; j++){
                list_elem curr = {};
                list_pop_back(ths->data[i], &curr);
                free(curr.key);
            }
            Delete_List(ths->data[i]);
        }
    }
}

void hash_map_destructor(Hash_map* ths){
    // for(size_t i = 0; size_t i < ths->)
    free_buckets(ths);
    free(ths->data);
}


static inline void update_load_factor(Hash_map* ths){
    ths->load_factor = ((double)(ths->size))/((double)(ths->bucket_count));
}

static unsigned long long hash(const char* line){
    
    unsigned long long hash = 5381;
    int c = 0;

    while (c = *line){
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        line++;
    }

    return hash;
}


static void resize(Hash_map* ths){
    List_for_map** new_data   = (List_for_map**)calloc(ths->bucket_count*2, sizeof(List_for_map*));

    size_t count = ths->bucket_count;
    ths->bucket_count = ths->bucket_count*2;

    for(size_t i = 0; i < count; i++){
        
        if(ths->data[i]){
            const size_t curr_count = ths->data[i]->size;
            for(int j = 0; j < curr_count; j++){

                Pair curr = {};
                list_pop_back(ths->data[i], &curr);

                size_t index = hash(curr.key)%(ths->bucket_count);

                if(new_data[index]){
                    list_push_front(new_data[index], curr);

                }else{
                    new_data[index] = New_List();

                    list_push_front(new_data[index], curr);
                }

            }
            Delete_List(ths->data[i]);

        }
    }
    free(ths->data);
    ths->data = new_data;

    update_load_factor(ths);


}




bool hash_map_insert(Hash_map* ths, const char* key, char* value){
    if(key == NULL){
        return false;
    }

    size_t index = hash(key)%(ths->bucket_count);
    
    if(hash_map_get(ths, key)){
        return false;
    }

    char* _key = (char*)calloc(strlen(key) + 1, sizeof(char));
    strcpy(_key, key);

    if(ths->data[index]){
        list_push_front(ths->data[index], {_key, value});

    }else{
        ths->data[index] = New_List();

        list_push_front(ths->data[index], {_key, value});
    }
    
    ths->size++;
    update_load_factor(ths);
    
    if(ths->load_factor > max_load_factor){
        resize(ths);
    }
    return true;
}


char* hash_map_get(Hash_map* ths, const char* key){
    size_t index = hash(key)%(ths->bucket_count);
    
    List_for_map* ptr = ths->data[index];

    if(!ptr){
        return NULL;
    }

    size_t count = ptr->size;

    for(size_t i = 1; i <= count; i++){
        Pair curr = {};
        list_get_value_by_index(ptr, i, &curr);

        if(strcmp(key, curr.key) == 0){
            return curr.value;
        }
    }
    
    return NULL;
}

