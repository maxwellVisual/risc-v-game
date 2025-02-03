#include "map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#define MAP_INIT_CAPACITY 16

int map_init(map_t* map, comparator_func_t* comparator, hash_func_t* hash){
    assert(map != NULL);
    assert(comparator != NULL);
    assert(hash != NULL);
    map->size = 0;
    map->comparator = comparator;
    map->hash = hash;
    map->capacity = MAP_INIT_CAPACITY;
    map->array = (map_node_t**)malloc(map->capacity * sizeof(map_node_t*));
    if(map->array == NULL) return 1;
    (void)memset(map->array, 0, map->capacity * sizeof(map_node_t*));
    return 0;
}

int reserve(map_t* map, size_t new_size){
    if (new_size < MAP_INIT_CAPACITY) {
        new_size = MAP_INIT_CAPACITY;
    }
    map_node_t** new_array = (map_node_t**)malloc(new_size * sizeof(map_node_t*));
    if (new_array == NULL) {
        return -1;
    }
    (void)memset(new_array, 0, new_size * sizeof(map_node_t*));

    for (size_t i = 0; i < map->capacity; ++i) {
        map_node_t* node = map->array[i];
        while (node) {
            map_node_t* next = node->next;
            size_t index = node->hash % new_size;
            node->next = new_array[index];
            new_array[index] = node;
            node = next;
        }
    }

    free(map->array);
    map->array = new_array;
    map->capacity = new_size;

    return 0;
}

void map_destroy(map_t* map){
    if(map->array != NULL) free(map->array);
}

void* map_remove(map_t* map, void* key, void* value){
    if(map->size < map->capacity / 4){
        if(reserve(map, map->capacity / 2)) return NULL;
    }
    map_node_t* node = (map->array[map->hash(key) % map->capacity]);
    if(node == NULL) return NULL;
    map_node_t* prev = NULL;
    while(node != NULL){
        if(node->key == key) break;
        if(value != NULL && 0 == map->comparator(node->value, value)) break;
        prev = node;
        node = node->next;
    }
    if(node == NULL) return NULL;
    if(prev != NULL) prev->next = node->next;
    else map->array[map->hash(key) % map->capacity] = node->next;
    map->size--;
    value = node->value;
    free(node);
    return value;
}
static map_node_t* map_get_node(map_t* map, void* key){
    map_node_t* node = (map->array[map->hash(key) % map->capacity]);
    if(node == NULL) return NULL;
    while(node != NULL){
        if(map->comparator(node->key, key) == 0) return node;
        node = node->next;
    }
    return NULL;
}
void* map_get(map_t* map, void* key){
    return map_get_node(map, key)->value;
}
void* map_set(map_t* map, void* key, void* value){
    if(map->size >= map->capacity){
        if(reserve(map, map->capacity * 2)) return NULL;
    }
    size_t index = map->hash(key) % map->capacity;
    map_node_t* node = (map->array[index]);

    while (node != NULL) {
        if (map->comparator(node->key, key) == 0) {
            // replace
            void* old_value = node->value;
            node->value = value;
            return old_value;
        }
        node = node->next;
    }
    // append
    node = (map_node_t*)malloc(sizeof(map_node_t));
    node->hash = map->hash(key);
    node->key = key;
    node->value = value;
    node->next = map->array[index];
    map->array[index] = node;
    map->size++;
    return value;
}
