
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "vm.h"
#include "error.h"
#include "elfutil.h"

static VM_t vm = {0};
static pid_t pid = 0;

void exit_handler(int status, void *args){
    VM_Deinit(&vm);
    if(pid != 0){
        kill(pid, SIGTERM);
        pid = 0;
    }
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
        ERROR_LOG("VM_Init failed: %d\n", ret);
        return ret;
    }
    vm.tick_rate = 0;
    FILE* elf = fopen("../demo/helloworld/hello.elf", "rb");
    if(elf == NULL){
        ERROR_LOG("elf not found\n");
        return 1;
    }
    ret = install_elf(elf, &vm);
    (void)fclose(elf);
    if(ret != 0) return 1;

    int pipedes[2];
    if(pipe(pipedes)){
        ERROR_LOG("pipe failed\n");
        return 1;
    }
    pid = fork();
    if(pid < 0){
        ERROR_LOG("fork failed\n");
        return 1;
    }else if(pid == 0){
        // new process
        dup2(pipedes[1], STDOUT_FILENO);
        close(pipedes[0]);
        ret = VM_Run(&vm);
        exit(ret);
    }else{
        // old process
        close(pipedes[1]);
        const char target[] = "hello risc-v\n";
        for (size_t i = 0; i < sizeof(target) - 1; i++)
        {
            char c;
            int ret = read(pipedes[0], &c, 1);
            if(ret != 1){
                ERROR_LOG("read failed\n");
                return 1;
            }
            if(c != target[i]){
                ERROR_LOG("wrong string\n");
                return 1;
            }
        }

        if(!kill(pid, SIGTERM)){
            pid = 0;
        }
    }
    return 0;
}