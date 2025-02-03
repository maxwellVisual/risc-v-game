#include <stdio.h>
#include <elf.h>
#include <assert.h>
#include <stdlib.h>

#include "elfutil.h"
#include "error.h"

FILE *elf = NULL;
FILE *expected_elf = NULL;

void exit_handler(int status, void *args){
    if(elf != NULL){
        fclose(elf);
    }
    if(expected_elf != NULL){
        fclose(expected_elf);
    }
    exit(status);
}

int main(int argc, char const *argv[])
{
    init_global_error_handler();
    int ret = 0;
    elf = fopen64("../demo/helloworld/hello.elf", "r");
    assert(elf != NULL);

    char* bin = NULL;
    size_t bin_size = elf2bin(elf, (void**)&bin);
    assert(0 != bin_size && bin != NULL);

    expected_elf = fopen("../demo/helloworld/hello.bin", "r");
    char c;
    size_t i;
    for (i = 0; i < bin_size; i++)
    {
        c = getc(expected_elf);
        assert(c == bin[i]);
    }
    c = getc(expected_elf);
    assert(c == EOF);

    return ret;
}
