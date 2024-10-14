#pragma once

#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>

typedef struct{
    size_t size;
    union{
        struct{
            FILE* file;
            off_t offset;
        };
        struct{
            uint8_t* memory;
        };
    };
    void (*get)(void* buffer, size_t offset, uint8_t bytes);
    void (*set)(size_t offset, void* data, uint8_t bytes);
    uint8_t is_internal;
} MemoryHandle;
int initInternalMemoryHandle(MemoryHandle* hmem, size_t size);
int initExternalMemoryHandle(MemoryHandle* hmem, size_t size, const char* file_name);

int destroyInternalMemoryHandle(MemoryHandle* hmem);
int destroyExternalMemoryHandle(MemoryHandle* hmem);

/* write data to memory */
void memory_set_u8(uint8_t data, size_t offset);
void memory_set_u16(uint16_t data, size_t offset);
void memory_set_u32(uint32_t data, size_t offset);
void memory_set_u64(uint64_t data, size_t offset);
void memory_set(size_t offset, void *data, uint8_t bytes);
/* read data from memory */
uint8_t memory_get_u8(size_t offset);
uint16_t memory_get_u16(size_t offset);
uint32_t memory_get_u32(size_t offset);
uint64_t memory_get_u64(size_t offset);
void memory_get(void *buffer, size_t offset, uint8_t bytes);

void memory_init(const char *_memory_file_name);
void memory_deinit(void);