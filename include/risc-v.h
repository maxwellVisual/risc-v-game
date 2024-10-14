#pragma once

#include <stdint.h>

#ifdef CONFIG_USE_RV_E
#   define RV_GP_REGISTERS_COUNT 16
#else
#   define RV_GP_REGISTERS_COUNT 32
#endif

#ifdef CONFIG_USE_RV64
typedef uint64_t RV_Uint;
typedef int64_t RV_Int;
#else
typedef uint32_t RV_Uint;
typedef int32_t RV_Int;
#endif

typedef RV_Uint RV_Register;
typedef RV_Uint RV_Ptr;
typedef uint8_t RV_RegisterIndex;
typedef uint32_t RV_Cmd;
typedef void (*RV_MemoryLoader)(void *dest, RV_Ptr src, uint8_t bytes);
typedef void (*RV_MemoryStorer)(RV_Ptr dest, void *src, uint8_t bytes);

typedef struct
{
    RV_Register regs[RV_GP_REGISTERS_COUNT];
    RV_Register reg_pc;
    RV_MemoryLoader mem_load;
    RV_MemoryStorer mem_store;
} RV_Env;

extern void RV_Exec(RV_Env *vm);
