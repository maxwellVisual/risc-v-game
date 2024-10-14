#include "memory.h"

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include "error.h"

#ifdef CONFIG_INTERNAL_MEMORY
#define __MEMORY_ARRAY_SIZE_BASE (CONFIG_INTERNAL_MEMORY / sizeof(uint32_t))
#define __MEMORY_ARRAY_SIZE_OVERFLOW ((CONFIG_INTERNAL_MEMORY % sizeof(uint32_t) == 0) ? 0 : 1)
#define MEMORY_SIZE __MEMORY_ARRAY_SIZE_BASE + __MEMORY_ARRAY_SIZE_OVERFLOW
static uint32_t memory_array[MEMORY_SIZE];
#else  // CONFIG_INTERNAL_MEMORY
static int memory_file = -1;
static const char *memory_file_name = NULL;
#endif // CONFIG_INTERNAL_MEMORY

#ifdef CONFIG_INTERNAL_MEMORY

#define _memory_set_data(bits)                                            \
    void memory_set_u##bits(uint##bits##_t data, size_t offset)           \
    {                                                                     \
        *((uint##bits##_t *)(((uint8_t *)memory_array) + offset)) = data; \
    }
#define _memory_get_data(bits)                                            \
    uint##bits##_t memory_get_u##bits(size_t offset)                      \
    {                                                                     \
        return *((uint##bits##_t *)(((uint8_t *)memory_array) + offset)); \
    }
void memory_get(void *buffer, size_t offset, uint8_t bytes){
    switch(bytes){
        case 1:
            *(uint8_t*)buffer = memory_get_u8(offset);
            break;
        case 2:
            *(uint16_t*)buffer = memory_get_u16(offset);
            break;
        case 4:
            *(uint32_t*)buffer = memory_get_u32(offset);
            break;
        case 8:
            *(uint64_t*)buffer = memory_get_u64(offset);
            break;
        default:
            error_handler();
    }
}
void memory_set(size_t offset, void *data, uint8_t bytes){
    switch(bytes){
        case 1:
            memory_set_u8(*(uint8_t*)data, offset);
            break;
        case 2:
            memory_set_u16(*(uint16_t*)data, offset);
            break;
        case 4:
            memory_set_u32(*(uint32_t*)data, offset);
            break;
        case 8:
            memory_set_u64(*(uint64_t*)data, offset);
            break;
        default:
            fprintf(stderr, "trying to write %d bytes to memory, which is not allowed.\n", bytes);
            error_handler();
    }
}
// /* unusable */
// int initInternalMemoryHandle(MemoryHandle* hmem, size_t size){
//     (void)size;
//     hmem->size = CONFIG_INTERNAL_MEMORY;
//     hmem->get = memory_get;
//     hmem->set = memory_set;
//     hmem->is_internal = 1;
//     hmem->memory = memory_array;
// }
// /* unusable */
// int destroyInternalMemoryHandle(MemoryHandle* hmem){
// }

#else // CONFIG_INTERNAL_MEMORY
static inline void force_memory_file_open(void)
{
    if (__glibc_unlikely(memory_file == -1))
    {
        if (__glibc_unlikely(memory_file_name == NULL))
        {
            error_handler();
        }
        memory_file = open(memory_file_name, O_RDWR | O_CREAT | O_RSYNC | O_NONBLOCK, 0600);
        if (__glibc_unlikely(memory_file < 0))
        {
            error_handler();
        }
#ifdef CONFIG_DEFAULT_MEMORY_FILE_SIZE
        struct stat64 stat;
        if (__glibc_unlikely(0 > fstat64(memory_file, &stat)))
        {
            error_handler();
        }
        if (__glibc_likely(stat.st_size > 0))
        {
            return;
        }
        if (__glibc_unlikely(0 > ftruncate64(memory_file, CONFIG_DEFAULT_MEMORY_FILE_SIZE)))
        {
            error_handler();
        }
#endif // CONFIG_DEFAULT_MEMORY_FILE_SIZE
    }
}
void memory_set(size_t offset, void *data, uint8_t bytes)
{
    force_memory_file_open();
    if (__glibc_unlikely(0 > pwrite64(memory_file, data, bytes, offset)))
    {
        error_handler();
    }
}
void memory_get(void *buffer, size_t offset, uint8_t bytes)
{
    force_memory_file_open();
    if (__glibc_unlikely(0 > pread64(memory_file, buffer, bytes, offset)))
    {
        error_handler();
    }
}

#define _memory_set_data(bits)                                      \
    void memory_set_u##bits(uint##bits##_t data, size_t offset)     \
    {                                                               \
        force_memory_file_open();                                   \
        memory_set(offset, (uint64_t*)&data, bits / 8); \
    }
#define _memory_get_data(bits)                            \
    uint##bits##_t memory_get_u##bits(size_t offset)      \
    {                                                     \
        uint##bits##_t buf;                               \
        force_memory_file_open();                         \
        memory_get(&buf, offset, bits / 8); \
        return buf;                                       \
    }
// /* unusable */
// int initExternalMemoryHandle(MemoryHandle* hmem, size_t size, const char* file_name){
//     hmem->file = fopen(file_name, "r");
//     memory_file = hmem->file->_fileno;
//     hmem->get = memory_get;
//     hmem->set = memory_set;
//     hmem->is_internal = 0;
//     hmem->offset = 0;
//     #ifdef CONFIG_DEFAULT_MEMORY_FILE_SIZE
//     hmem->size = CONFIG_DEFAULT_MEMORY_FILE_SIZE;
//     #else
//     hmem->size = size;
//     #endif
// }
// /* unusable */
// int destroyExternalMemoryHandle(MemoryHandle* hmem){
    
// }

#endif // CONFIG_INTERNAL_MEMORY

/* write data to memory */
_memory_set_data(8)
_memory_set_data(16)
_memory_set_data(32)
_memory_set_data(64)
/* read data from memory */
_memory_get_data(8)
_memory_get_data(16)
_memory_get_data(32)
_memory_get_data(64)

void memory_init(const char *_memory_file_name)
{
#ifndef CONFIG_INTERNAL_MEMORY
    register uint16_t len = strnlen(_memory_file_name, UINT16_MAX);
    memory_file_name = (const char *)malloc(len * sizeof(char));
    (void)memcpy((void *)memory_file_name, (void *)_memory_file_name, len);

    force_memory_file_open();
#else
    if (NULL == memset(memory_array, 0, CONFIG_INTERNAL_MEMORY))
    {
        error_handler();
    }
#endif // CONFIG_INTERNAL_MEMORY
}
void memory_deinit(void)
{
#ifndef CONFIG_INTERNAL_MEMORY
    if (memory_file > 0)
    {
        close(memory_file);
        memory_file = -1;
    }
    if (memory_file_name != NULL)
    {
        free((void *)memory_file_name);
        memory_file_name = NULL;
    }
#endif // CONFIG_INTERNAL_MEMORY
}