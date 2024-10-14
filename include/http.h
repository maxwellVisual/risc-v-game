#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef struct{
    const char* raw_args;
    const char* path;
    size_t content_size;
    char content[];
}HTTP_Msg;

typedef struct{
    const char* raw_args;
    uint16_t code;
    size_t content_size;
    char content[];
}HTTP_Result;
