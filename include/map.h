#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#define MAP_INIT_CAPACITY 16

typedef int comparator_func_t(void* a, void* b);// a - b
typedef uint32_t hash_t;
typedef hash_t hash_func_t(void* key);

typedef struct _map_node_t{
    hash_t hash;
    void* key;
    void* value;
    struct _map_node_t* next;
} map_node_t;
typedef struct{
    size_t size;
    comparator_func_t* comparator;
    hash_func_t* hash;
    map_node_t** array;
    size_t capacity;
} map_t;

extern int map_init(map_t* map, comparator_func_t* comparator, hash_func_t* hash);
extern void map_destroy(map_t* map);
extern void* map_remove(map_t* map, void* key, void* value);
extern void* map_get(map_t* map, void* key);
extern void* map_set(map_t* map, void* key, void* value);
extern int reserve(map_t* map, size_t new_size);
