#pragma once

#include <stdio.h>

#include "risc-v.h"
#include "memory.h"
#include "user_group.h"

#define VM_DEFAULT_TICK_RATE ((useconds_t)10000)

typedef struct{
    privileged_mem_t mem;
    RV_Env** vms;
    size_t vms_size;
    size_t vm_count;
    uint64_t tick;
    FILE* persistent_mem_file;
    privileged_group_map_t groups_set;
    privilege_id_t max_uid;
    privilege_id_t cur_uid;
    useconds_t tick_rate;
}VM_t;

int VM_Init(VM_t* vm, privileged_mem_factory* mem_factory, const char* mem_file_name);
int VM_Run(VM_t* vm);
void VM_Deinit(VM_t* vm);

