#include "check_config.h"

#include <stdio.h>
#include <stdint.h>
#define RAND_MAX UINT8_MAX
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#define FILE_NAME "world.img.test"

#include "memory.h"
#include "error.h"


void error_handler(void){
    exit(2);
}
void exit_handler(int status, void *args){
    memory_deinit();
    (void)remove(FILE_NAME);
    exit(status);
}
void signal_handler(int signal){
    exit(1);
}
int main(int argc, char const *argv[])
{
    uint32_t memory_mirror[1024/sizeof(uint32_t)];
    init_error_handler();
    memory_init(FILE_NAME);
    srand((unsigned int)time(NULL));
    for(uint32_t i=0; i<1024/sizeof(uint32_t); i++){
        uint32_t data = 0;
        for(int j=0; j<4; j++){
            data <<= 8;
            data |= (uint8_t)rand();
        }
        memory_set_u32(data, i*sizeof(uint32_t));
        memory_mirror[i] = data;
    }

    for(int i=0; i<1024/sizeof(uint8_t); i++){
        uint8_t buf = memory_get_u8(i * sizeof(uint8_t));
        uint8_t mirror = ((uint8_t*)memory_mirror)[i];
        assert(buf == mirror);
    }
    for(int i=0; i<1024/sizeof(uint16_t); i++){
        uint16_t buf = memory_get_u16(i * sizeof(uint16_t));
        assert(buf == ((uint16_t*)memory_mirror)[i]);
    }
    for(int i=0; i<1024/sizeof(uint32_t); i++){
        uint32_t buf = memory_get_u32(i * sizeof(uint32_t));
        assert(buf == ((uint32_t*)memory_mirror)[i]);
    }
    for(int i=0; i<1024/sizeof(uint64_t); i++){
        uint64_t buf = memory_get_u64(i * sizeof(uint64_t));
        assert(buf == ((uint64_t*)memory_mirror)[i]);
    }

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
            switch(size){
                case 1:
                    memory_set_u8((uint8_t)data, offset);
                    *((uint8_t*)(((uint8_t*)memory_mirror)+offset)) = (uint8_t)data;
                    break;
                case 2:
                    memory_set_u16((uint16_t)data, offset);
                    *((uint16_t*)(((uint8_t*)memory_mirror)+offset)) = (uint16_t)data;
                    break;
                case 4:
                    memory_set_u32((uint32_t)data, offset);
                    *((uint32_t*)(((uint8_t*)memory_mirror)+offset)) = (uint32_t)data;
                    break;
                case 8:
                    memory_set_u64((uint64_t)data, offset);
                    *((uint64_t*)(((uint8_t*)memory_mirror)+offset)) = (uint64_t)data;
                    break;
            }
        }else{
            /* get */
            switch(size){
                case 1:
                    uint8_t buf1 = memory_get_u8(offset);
                    uint8_t mirror1 = ((uint8_t*)memory_mirror)[offset];
                    if(buf1 != mirror1){
                        printf("%x %x\n", buf1, mirror1);
                    }
                    assert(buf1 == mirror1);
                    break;
                case 2:
                    uint16_t buf2 = memory_get_u16(offset);
                    uint16_t mirror2 = *(uint16_t*)(((uint8_t*)memory_mirror)+offset);
                    if(buf2 != mirror2){
                        printf("%x %x\n", buf2, mirror2);
                    }
                    assert(buf2 == mirror2);
                    break;
                case 4:
                    uint32_t buf4 = memory_get_u32(offset);
                    uint32_t mirror4 = *(uint32_t*)(((uint8_t*)memory_mirror)+offset);
                    if(buf4 != mirror4){
                        printf("%x %x\n", buf4, mirror4);
                    }
                    assert(buf4 == mirror4);
                    break;
                case 8:
                    uint64_t buf8 = memory_get_u64(offset);
                    uint64_t mirror8 = *(uint64_t*)(((uint8_t*)memory_mirror)+offset);
                    if(buf8 != mirror8){
                        printf("%lx %lx\n", buf8, mirror8);
                    }
                    assert(buf8 == mirror8);
                    break;
            }
        }
        count++;
    }

    // memory_deinit();
    return 0;
}
