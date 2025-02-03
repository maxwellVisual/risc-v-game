#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "privilege.h"
#include "error.h"

#define PRIVILEGED_MEMORY_FILE_MAGNUM ((uint8_t[]){0x7f, 'P', 'M', 'N'})

typedef struct{
    uint8_t magic_number[4];
    uint64_t block_count;
    uint64_t block_size;
    uint64_t reserved[2];
} privileged_mem_file_hdr;

// typedef enum{
//     PRIVILEGED_MEM_TYPE_FILE,
//     PRIVILEGED_MEM_TYPE_MEMORY
// } privileged_mem_type_t;

typedef struct{
    uint8_t* mem;
    size_t block_count;
    size_t block_size;
    privilege_entry_t* block_privileges;
    uint8_t flags;
} privileged_mem_t;

typedef struct{
    size_t block_count;
    size_t block_size;
    privilege_entry_t default_privilege;
} privileged_mem_factory;

extern int memory_init(privileged_mem_factory* factory, privileged_mem_t* mem);
extern void memory_deinit(privileged_mem_t* mem);
/**
 * file format:
 * privileged_mem_file_hdr
 * mem (block_count * block_size bytes)
 * privilege_entries (block_count * 32 bytes)
 * [EOF]
 */
extern void memory_save(privileged_mem_t* mem, FILE* dest);
extern void memory_load(privileged_mem_t* mem, FILE* src);

/**
 * @returns 0 when succeed and 1 when writing without privilege
 */
extern int memory_set(privileged_mem_t* mem, privilege_id_t user_id, size_t offset, void* data, size_t bytes);
extern int memory_get(privileged_mem_t* mem, privilege_id_t user_id, void* buffer, size_t offset, size_t bytes);

extern int memory_can_set(privileged_mem_t* mem, privilege_id_t user_id, size_t offset, size_t bytes);
extern int memory_can_get(privileged_mem_t* mem, privilege_id_t user_id, size_t offset, size_t bytes);
extern int memory_can_exec(privileged_mem_t* mem, privilege_id_t user_id, size_t offset, size_t bytes);

extern int memory_has_changed(privileged_mem_t* mem);
