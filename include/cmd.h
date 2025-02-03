#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct opts{
    const char* cmd;
    void (*handler)(size_t argc, const char** argv);
    struct opts** sub_opts;
};

typedef struct opts Opts;

int parse_opts(char* args, Opts **opts);
