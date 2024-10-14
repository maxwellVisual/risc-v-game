#include "elfutil.h"

#include <stdio.h>
#include <elf.h>
#include <string.h>
#include <stdlib.h>

#include "error.h"

/* elf validity check */
static inline int is_elf_valid(Elf32_Ehdr *header)
{
    /* check magic number */
    if (memcmp(header->e_ident, ELFMAG, sizeof(ELFMAG) - 1))
    {
        ERROR_LOG("not an elf file\n");
        return 0;
    }
    /* check if is executable elf */
    if (header->e_type != ET_EXEC)
    {
        ERROR_LOG("not an elf executable\n");
        return 0;
    }
    /* check if is riscv elf */
    if (header->e_machine != EM_RISCV)
    {
        ERROR_LOG("not a risc-v executable\n");
        return 0;
    }
    /* check version */
    if (header->e_version != EV_CURRENT)
    {
        ERROR_LOG("elf version error\n");
        return 0;
    }
    /* check machine flags */
    if (header->e_flags != 0)
    {
        ERROR_LOG("wrong machine flags\n");
        return 0;
    }
    return 1;
}
int read_data(FILE *input, void *dest, size_t offset, size_t size, size_t n)
{
    if(size == 0 || n == 0){
        return 0;
    }
    if (0 != fseeko64(input, offset, SEEK_SET))
    {
        return 1;
    }
    if (n > fread(dest, size, n, input))
    {
        return 1;
    }
    return 0;
}

/* @returns size of binary */
size_t elf2bin(FILE *input, void **buffer)
{
    Elf32_Ehdr elf_header = {0};
    if (read_data(input, &elf_header, 0, sizeof(Elf32_Ehdr), 1))
    {
        ERROR_LOG("can't get elf header\n");
        return 0;
    }
    if (!is_elf_valid(&elf_header))
    {
        return 0;
    }

    // /* handle segment header array */
    // Elf32_Shdr segment_header[elf_header.e_shnum];
    // if (read_data(input, segment_header, elf_header.e_shoff, elf_header.e_shentsize, elf_header.e_shnum))
    // {
    //     ERROR_LOG("can't get segment headers\n");
    //     return 0;
    // }

    // Elf32_Shdr *strtab_header = &segment_header[elf_header.e_shstrndx];
    // char strtab[strtab_header->sh_size];
    // if (read_data(input, strtab, strtab_header->sh_offset, sizeof(unsigned char), strtab_header->sh_size))
    // {
    //     ERROR_LOG("can't get string table\n");
    //     return 0;
    // }

    // Elf32_Shdr *symtab_header = NULL;
    // for (int i = 0; i < elf_header.e_shnum; i++)
    // {
    //     Elf32_Word type = segment_header[i].sh_type;
    //     if (type == SHT_SYMTAB || type == SHT_DYNSYM)
    //     {
    //         symtab_header = &segment_header[i];
    //         break;
    //     }
    // }
    // if (symtab_header == NULL)
    // {
    //     ERROR_LOG("symbol table not found\n");
    //     return 0;
    // }

    // Elf32_Off sh_offset = symtab_header->sh_offset;
    // Elf32_Word sh_entsize = symtab_header->sh_entsize;
    // Elf32_Word sh_length = symtab_header->sh_size / sh_entsize;

    // Elf32_Sym symtab[sh_length];
    // if (read_data(input, symtab, sh_offset, sh_entsize, sh_length))
    // {
    //     ERROR_LOG("can't read from symbol table\n");
    //     return 0;
    // }

    Elf32_Phdr program_header[elf_header.e_phnum];
    if (read_data(input, &program_header, elf_header.e_phoff, elf_header.e_phentsize, elf_header.e_phnum))
    {
        ERROR_LOG("can't read program header\n");
        return 0;
    }

    size_t program_size = 0;

    for (size_t i = 0; i < elf_header.e_phnum; i++)
    {
        if (program_header[i].p_type != PT_LOAD)
        {
            continue;
        }
        size_t top = program_header[i].p_filesz + program_header[i].p_vaddr;
        if (top > program_size)
        {
            program_size = top;
        }
    }
    if (program_size == 0)
    {
        *buffer = NULL;
        return 0;
    }

    char *data = malloc(program_size);
    (void)memset(data, 0, program_size);

    for (size_t i = 0; i < elf_header.e_phnum; i++)
    {
        Elf32_Phdr *header = &program_header[i];
        if (header->p_type != PT_LOAD)
        {
            continue;
        }
        if (read_data(input, data + header->p_vaddr, header->p_offset, header->p_filesz, 1))
        {
            ERROR_LOG("can't read from program header\n");
            free(data);
            *buffer = NULL;
            return 0;
        }
    }

    *buffer = data;
    return program_size;
}