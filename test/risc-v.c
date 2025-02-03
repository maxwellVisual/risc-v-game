#include "check_config.h"

#include <stdio.h>
#include <stdint.h>
#define RAND_MAX UINT8_MAX
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#include "memory.h"
#include "error.h"
#include "risc-v.h"
#include "memory.h"

static FILE *bin = NULL;
static privileged_mem_factory factory = {
    .block_size = 1024,
    .block_count = 128,
};
static privileged_mem_t mem = {0};

void error_handler(void){
    exit(2);
}
void exit_handler(int status, void *args){
    if(status > 0){
        printf("exit: %d\n", status);
    }
    if(bin != NULL){
        (void)fclose(bin);
        bin = NULL;
    }
    memory_deinit(&mem);
    exit(status);
}
void signal_handler(int signal){
    exit(1);
}

static int mem_load(RV_Env *this, void *dest, RV_Ptr src, uint8_t bytes){
    return memory_get(&mem, 0, dest, (size_t)src, bytes);
}
static int mem_store(RV_Env *this, RV_Ptr dest, void *src, uint8_t bytes){
    return memory_set(&mem, 0, dest, src, bytes);
}

int main(int argc, char const *argv[])
{
    int ret = 0;
    init_global_error_handler();
    memory_init(&factory, &mem);

    bin = fopen("../demo/helloworld/hello.bin", "r");
    if(bin == NULL){
        ret = 1;
        goto memory_deinit;
    }

    RV_Env vm;
    vm.mem_load = mem_load;
    vm.mem_store = mem_store;
    vm.reg_pc = 0x00;
    for(int i=0; i<sizeof(vm.regs)/sizeof(*(vm.regs)); i++){
        vm.regs[i] = 0;
    }
    for(int i=0;; i+=4){
        RV_Cmd cmd;
        size_t size = fread(&cmd, 4, 1, bin);
        if(size < 1){
            break;
        }
        vm.mem_store(&vm, i, &cmd, 4);
    }
    (void)fclose(bin);
    bin = NULL;
    char target[] = "hello risc-v\n";
    char* target_ptr = target;
    while(1){
        RV_Cmd cmd = 0;
        vm.mem_load(&vm, &cmd, (RV_Ptr)vm.reg_pc, sizeof(RV_Cmd));
        if(vm.reg_pc == 0x10){// exit:
            break;
        }
        if(cmd == 0){
            char c = vm.regs[11];
            if(*target_ptr != c){
                ret = 1;
                goto memory_deinit;
            }else if(*target_ptr == '\0'){
                ret = 1;
                goto memory_deinit;
            }
            target_ptr++;
            vm.reg_pc+=4;
            continue;
        }
        RV_Exec(&vm);
    }
    if(*target_ptr != '\0'){
        ret = 1;
    }
memory_deinit:
    memory_deinit(&mem);
    return ret;
}
