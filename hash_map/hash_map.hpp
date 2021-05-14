#pragma once
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "list.hpp"

const double max_load_factor = 0.75;


struct Hash_map{
    List_for_map** data;

    long long size;
    long long bucket_count;

    double load_factor;

};

void hash_map_constructor(Hash_map* ths);

void hash_map_destructor(Hash_map* ths);

bool hash_map_insert(Hash_map* ths, const char* key, char* value);

char* hash_map_get(Hash_map* ths, const char* key);

