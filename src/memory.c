#include "memory.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "privilege.h"
#include "error.h"

#define MEM_FLAGS_CHANGED 0x01u
#define IS_MEM_CHAMGED(flags) ((flags) & MEM_FLAGS_CHANGED)
#define SET_MEM_CHANGED(flags) ((flags) |= MEM_FLAGS_CHANGED)
#define CLEAR_MEM_CHANGED(flags) ((flags) &= ~MEM_FLAGS_CHANGED)

int memory_init(privileged_mem_factory* factory, privileged_mem_t* mem){
    mem->block_size = factory->block_size;
    mem->block_count = factory->block_count;
    mem->mem = (uint8_t*)malloc(mem->block_count * mem->block_size);
    if(mem->mem == NULL){
        return 1;
    }
    mem->block_privileges = (privilege_entry_t*)calloc(mem->block_count, sizeof(privilege_entry_t));
    if(mem->block_privileges == NULL){
        free(mem->mem);
        return 1;
    }
    for(size_t i=0; i<mem->block_count; i++){
        (void)memcpy(&mem->block_privileges[i], &factory->default_privilege, sizeof(privilege_entry_t));
    }
    return 0;
}

void memory_deinit(privileged_mem_t* mem){
    free(mem->mem);
    free(mem->block_privileges);
    mem->mem = NULL;
    mem->block_privileges = NULL;
}

void memory_save(privileged_mem_t* mem, FILE* dest){
    assert(dest != NULL);

    privileged_mem_file_hdr header;
    (void)memcpy(header.magic_number, PRIVILEGED_MEMORY_FILE_MAGNUM, 4);
    header.block_count = mem->block_count;
    header.block_size = mem->block_size;
    (void)memcpy(header.reserved, (uint64_t[]){0,0}, 2);
    uint64_t block_count = header.block_count;
    uint64_t block_entry_size = sizeof(privilege_entry_t);
    RT_ASSERT(0 == fseek(dest, 0, SEEK_SET))
    RT_ASSERT(1 == fwrite(&header, sizeof(privileged_mem_file_hdr), 1, dest));
    RT_ASSERT(block_count == fwrite(mem->mem, header.block_size, block_count, dest));
    RT_ASSERT(block_count == fwrite(mem->block_privileges, block_entry_size, block_count, dest));
}
void memory_load(privileged_mem_t* mem, FILE* src){
    assert(src != NULL);

    privileged_mem_file_hdr header;
    RT_ASSERT(0 == fseek(src, 0, SEEK_SET));
    RT_ASSERT(1 == fread(&header, sizeof(privileged_mem_file_hdr), 1, src));
    
    privileged_mem_factory factory;
    factory.block_count = header.block_count;
    factory.block_size = header.block_size;
    memory_init(&factory, mem);

    uint64_t block_count = header.block_count;
    uint64_t block_entry_size = sizeof(privilege_entry_t);

    RT_ASSERT(block_count == fread(mem->mem, header.block_size, block_count, src));
    RT_ASSERT(block_count == fread(mem->block_privileges, block_entry_size, block_count, src));
}

int memory_set(privileged_mem_t* mem, privilege_id_t user_id, size_t offset, void *data, size_t bytes){
    assert(data != NULL);
    size_t block_offset = offset/mem->block_size;
    privilege_entry_t* entry = &(mem->block_privileges[block_offset]);
    if(entry->users.write != user_id && entry->user_level.write > entry->base_level.write){
        return 2;
    }
    if(mem->block_count * mem->block_size <= offset + bytes){
        return 1;
    }
    (void)memcpy(&mem->mem[offset], data, bytes);
    SET_MEM_CHANGED(mem->flags);
    return 0;
}
int memory_get(privileged_mem_t* mem, privilege_id_t user_id, void *buffer, size_t offset, size_t bytes){
    assert(buffer != NULL);

    size_t block_offset = offset / mem->block_size;
    privilege_entry_t* entry = &(mem->block_privileges[block_offset]);
    if(entry->users.read != user_id && entry->user_level.read > entry->base_level.read){
        return 2;
    }
    if(mem->block_count * mem->block_size <= offset + bytes){
        return 1;
    }
    (void)memcpy(buffer, &mem->mem[offset], bytes);
    return 0;
}

int memory_can_set(privileged_mem_t* mem, privilege_id_t user_id, size_t offset, size_t bytes){
    if(bytes == 0){
        return 1;
    }
    size_t first_block = offset / mem->block_size;
    size_t last_block = (offset + bytes - 1) / mem->block_size;
    for(size_t i = first_block; i <= last_block; i++){
        privilege_entry_t* entry = &(mem->block_privileges[i]);
        if(entry->users.write != user_id && entry->user_level.write > entry->base_level.write){
            return 0;
        }
    }
    return 1;
}
int memory_can_get(privileged_mem_t* mem, privilege_id_t user_id, size_t offset, size_t bytes){
    if(bytes == 0){
        return 1;
    }
    size_t first_block = offset / mem->block_size;
    size_t last_block = (offset + bytes - 1) / mem->block_size;
    for(size_t i = first_block; i <= last_block; i++){
        privilege_entry_t* entry = &(mem->block_privileges[i]);
        if(entry->users.read != user_id && entry->user_level.read > entry->base_level.read){
            return 0;
        }
    }
    return 1;
}
int memory_can_exec(privileged_mem_t* mem, privilege_id_t user_id, size_t offset, size_t bytes){
    if(bytes == 0){
        return 1;
    }
    size_t first_block = offset / mem->block_size;
    size_t last_block = (offset + bytes - 1) / mem->block_size;
    for(size_t i = first_block; i <= last_block; i++){
        privilege_entry_t* entry = &(mem->block_privileges[i]);
        if(entry->users.exec != user_id && entry->user_level.exec > entry->base_level.exec){
            return 0;
        }
    }
    return 1;
}

int memory_has_changed(privileged_mem_t* mem){
    int ans = IS_MEM_CHAMGED(mem->flags);
    CLEAR_MEM_CHANGED(mem->flags);
    return ans;
}
