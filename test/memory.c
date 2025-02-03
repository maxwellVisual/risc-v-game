
#include "check_config.h"

#include <stdio.h>
#include <stdint.h>
#define RAND_MAX UINT8_MAX
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#define FILE_NAME "/tmp/world.img.test"

#include "memory.h"
#include "error.h"

static privileged_mem_factory factory = {
    .block_size = 1024,
    .block_count = 128,
    .default_privilege = {
        .base_level = {
            .read = UINT8_MAX,
            .write = UINT8_MAX,
            .exec = UINT8_MAX,
        }
    }
};
static privileged_mem_t mem = {0};

void error_handler(void){
    exit(2);
}
void exit_handler(int status, void *args){
    memory_deinit(&mem);
    (void)remove(FILE_NAME);
    exit(status);
}
void signal_handler(int signal){
    exit(1);
}
int main(int argc, char const *argv[])
{
    uint32_t memory_mirror[factory.block_size * factory.block_count/sizeof(uint32_t)];
    init_global_error_handler();
    memory_init(&factory, &mem);
    srand((unsigned int)time(NULL));
    for(uint32_t i=0; i<factory.block_count * factory.block_size/sizeof(uint32_t); i++){
        uint32_t data = 0;
        for(int j=0; j<4; j++){
            data <<= 8;
            data |= (uint8_t)rand();
        }
        memory_set(&mem, 0, i*sizeof(uint32_t), &data, sizeof(uint32_t));
        memory_mirror[i] = data;
    }
    FILE* file = fopen(FILE_NAME, "wb+");
    memory_save(&mem, file);
    memory_deinit(&mem);
    fclose(file);
    file = fopen(FILE_NAME, "r");
    memory_load(&mem, file);
    fclose(file);
    uint64_t count = 0;
    while(count < 10000){
        uint8_t r = rand();
        uint8_t size = 1 << (r & 0b11);
        uint32_t offset = ((((uint32_t)(uint8_t)rand())&0b11) << 8) | ((uint32_t)(uint8_t)rand());
        if(offset + size - 1 >= 1024){
            continue;
        }

        if(r & 0b100){
            /* set */
            uint64_t data = 0;
            for(int i=0; i<size; i++){
                data <<= 8;
                data |= (uint8_t)rand();
            }
            memory_set(&mem, 0, offset, &data, size);
            switch(size){
                case 1:
                    *((uint8_t*)(((uint8_t*)memory_mirror)+offset)) = (uint8_t)data;
                    break;
                case 2:
                    *((uint16_t*)(((uint8_t*)memory_mirror)+offset)) = (uint16_t)data;
                    break;
                case 4:
                    *((uint32_t*)(((uint8_t*)memory_mirror)+offset)) = (uint32_t)data;
                    break;
                case 8:
                    *((uint64_t*)(((uint8_t*)memory_mirror)+offset)) = (uint64_t)data;
                    break;
            }
        }else{
            /* get */
            uint64_t buf = 0;
            uint64_t mirror = 0;
            memory_get(&mem, 0, &buf, offset, size);
            switch(size){
                case 1:
                    mirror |= ((uint8_t*)memory_mirror)[offset];
                    break;
                case 2:
                    mirror |= *(uint16_t*)(((uint8_t*)memory_mirror)+offset);
                    break;
                case 4:
                    mirror |= *(uint32_t*)(((uint8_t*)memory_mirror)+offset);
                    break;
                case 8:
                    mirror |= *(uint64_t*)(((uint8_t*)memory_mirror)+offset);
                    break;
            }
            assert(buf == mirror);
        }
        count++;
    }

    memory_deinit(&mem);
    return 0;
}
