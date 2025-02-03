#pragma once

#include <stdio.h>

#include "vm.h"

size_t elf2bin(FILE* input, void** buffer);
int install_elf(FILE* elf, VM_t* vm);
