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

static FILE *bin = NULL;

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
    memory_deinit();
    exit(status);
}
void signal_handler(int signal){
    exit(1);
}

static void mem_load(void *dest, RV_Ptr src, uint8_t bytes){
    memory_get(dest, (size_t)src, bytes);
}
static void mem_store(RV_Ptr dest, void *src, uint8_t bytes){
    memory_set(dest, src, bytes);
}

int main(int argc, char const *argv[])
{
    int ret = 0;
    init_error_handler();
    memory_init("world.img");

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
        vm.mem_store(i, &cmd, 4);
    }
    (void)fclose(bin);
    bin = NULL;
    char target[] = "hello risc-v\n";
    char* target_ptr = target;
    while(1){
        RV_Cmd cmd = 0;
        vm.mem_load(&cmd, (RV_Ptr)vm.reg_pc, sizeof(RV_Cmd));
        if(vm.reg_pc == 0x10){
            break;
        }
        if(cmd == 0){
            RV_Ptr ptr = vm.regs[11];
            RV_Int len = vm.regs[12];
            if(len != sizeof(target) - 1){
                ret = 1;
                goto memory_deinit;
            }
            for(int i=0; i<len; i++){
                char c;
                vm.mem_load(&c, ptr+i, 1);
                if(*target_ptr != c){
                    ret = 1;
                    goto memory_deinit;
                }else if(*target_ptr == '\0'){
                    ret = 1;
                    goto memory_deinit;
                }
                // putchar(c);
                target_ptr++;
            }
            vm.reg_pc+=4;
            continue;
        }
        RV_Exec(&vm);
    }
    if(*target_ptr != '\0'){
        ret = 1;
    }
memory_deinit:
    memory_deinit();
    return ret;
}
