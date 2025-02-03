#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
    size_t count;
    size_t size;
    size_t capacity;
    void* data;
} array_t;

extern int array_push(array_t* arr, void* data);
extern int array_reserve(array_t* arr, size_t count);
extern int array_pop(array_t* arr);