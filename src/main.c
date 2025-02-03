
#include <stdio.h>
#include <stdlib.h>

#include "vm.h"
#include "error.h"
#include "elfutil.h"

static VM_t vm = {0};

void exit_handler(int status, void *args){
    VM_Deinit(&vm);
    exit(status);
}

int main(int argc, char const *argv[])
{
    int ret = 0;
    init_global_error_handler();
    ret = VM_Init(&vm, &(privileged_mem_factory){
        .block_size = 1024,
        .block_count = 128,
        .default_privilege = {
            .base_level = {
                .read = UINT8_MAX,
                .write = UINT8_MAX,
                .exec = UINT8_MAX,
            }
        }
    }, "world.bin");
    if(ret != 0){
        fprintf(stderr, "VM_Init failed: %d\n", ret);
        return ret;
    }
    vm.tick_rate = 1000;
    FILE* elf = fopen("/home/maxwell/programFiles/projects/risc-v-game/demo/helloworld/hello.elf", "rb");
    if(elf == NULL){
        fprintf(stderr, "elf not found\n");
        return 1;
    }
    ret = install_elf(elf, &vm);
    (void)fclose(elf);
    if(ret != 0) return 1;
    ret = VM_Run(&vm);
    return ret;
}
