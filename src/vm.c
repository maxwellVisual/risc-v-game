#include "vm.h"

#include <stdio.h>
#include <assert.h>
#include <elf.h>
#include <unistd.h>

#include "memory.h"
#include "risc-v.h"
#include "cmd.h"
#include "error.h"
#include "elfutil.h"
#include "arrays.h"
#include "user_group.h"

static FILE* create_persistent_file(const char *filename){
    FILE* file = fopen(filename, "rb+");
    if(file != NULL){
        return file;
    }
    file = fopen(filename, "wb+");
    if(file != NULL){
        return file;
    }
    return NULL;
}
static RV_Env* VM_add_env(VM_t* vm) {
    array_t arr = {
        .capacity = vm->vms_size,
        .count = vm->vm_count,
        .size = sizeof(RV_Env*),
        .data = vm->vms,
    };
    RV_Env* env = RV_Create(&vm->mem);
    int ret = array_push(&arr, &env);
    if(ret) return NULL;
    vm->vms = arr.data;
    vm->vms_size = arr.capacity;
    vm->vm_count = arr.count;
    return env;
}
static privileged_group_t* VM_add_user(VM_t* vm){
    privileged_group_t* group = privileged_groups_add_user(&vm->groups_set, vm->max_uid);
    RV_Env* env = VM_add_env(vm);
    if(env == NULL){
        privileged_groups_remove(&vm->groups_set, group->id);
        return NULL;
    }
    group->env = env;
    vm->max_uid++;
    return group;
}
int VM_Init(VM_t* vm, privileged_mem_factory* mem_factory, const char* mem_file_name)
{
    int ret = 0;
    ret = memory_init(mem_factory, &(vm->mem));
    if(ret != 0) goto err;

    vm->vms = NULL;
    vm->vms_size = 0;
    vm->vm_count = 0;
    vm->tick = 0;
    vm->tick_rate = VM_DEFAULT_TICK_RATE;

    vm->persistent_mem_file = create_persistent_file(mem_file_name);
    if(vm->persistent_mem_file == NULL) goto err_mem;

    ret = privileged_groups_init(&vm->groups_set);
    if(ret != 0) goto err_file;

    vm->max_uid = PRIVILEGED_GROUP_ROOT_ID;
    ret = NULL == VM_add_user(vm);
    if(ret != 0) goto err_groups;

    return 0;
err_groups:
    privileged_groups_deinit(&vm->groups_set);
err_file:
    (void)fclose(vm->persistent_mem_file);
err_mem:
    memory_deinit(&vm->mem);
err:
    return 1;
}

static int VM_step(VM_t* vm)
{
    for (size_t i = 1; i < vm->vm_count; i++)
    {
        vm->cur_uid = i;
        RV_Exec(vm->vms[i]);
    }
    vm->cur_uid = 0;
    RV_Exec(vm->vms[0]);
    return 0;
}
int VM_Run(VM_t* vm)
{
    while (!VM_step(vm)) {
        if(memory_has_changed(&vm->mem)){
            memory_save(&(vm->mem), vm->persistent_mem_file);
        }
        vm->tick++;
        if(vm->tick_rate > 0){
            (void)usleep(vm->tick_rate);
        }
    }
    return 0;
}
void VM_Deinit(VM_t* vm)
{
    memory_deinit(&(vm->mem));
    for (size_t i = 0; i < vm->vm_count; i++){
        free(vm->vms[i]);
    }
    free(vm->vms);
}
