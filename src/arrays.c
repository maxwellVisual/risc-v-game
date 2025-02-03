#include "arrays.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int array_push(array_t* arr, void* data){
    if(arr->count * arr->size >= arr->capacity){
        arr->capacity *= 2;
        arr->data = realloc(arr->data, arr->capacity);
    }
    if(arr->data == NULL){
        return 1;
    }
    if(NULL == memcpy((void*)((uint8_t*)arr->data + arr->count * arr->size), data, arr->size)){
        return 1;
    }
    arr->count++;
    return 0;
}
int array_reserve(array_t* arr, size_t count){
    uint16_t new_capacity = arr->capacity;
    if(new_capacity == 0){
        new_capacity = 1;
    }
    while(new_capacity < count * arr->size){
        new_capacity *= 2;
    }
    arr->data = realloc(arr->data, new_capacity);
    if(arr->data == NULL){
        return 1;
    }
    arr->capacity = new_capacity;
    return 0;
}
int array_pop(array_t* arr){
    if(arr->count == 0){
        return 1;
    }
    arr->count--;
    if(arr->count * arr->size < arr->capacity / 2){
        arr->capacity /= 2;
        arr->data = realloc(arr->data, arr->capacity);
    }
    if(arr->data == NULL){
        return 1;
    }
    return 0;
}