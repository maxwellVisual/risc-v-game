#pragma once

#include <stdint.h>
#include <stddef.h>

#include "map.h"

typedef uint64_t privilege_id_t;

typedef struct{
    uint8_t read;
    uint8_t write;
    uint8_t exec;
} privilege_level_t;

typedef struct{
    privilege_id_t read;
    privilege_id_t write;
    privilege_id_t exec;
} privilege_ownership_t;

typedef struct{
    privilege_level_t user_level;
    privilege_level_t group_level;
    privilege_ownership_t users;
    privilege_ownership_t groups;
    privilege_level_t base_level;// maximum level before being private
} privilege_entry_t;// unix-like
