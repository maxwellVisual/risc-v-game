#pragma once

#include <stdint.h>
#include <memory.h>

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

typedef struct _RV_Env
{
    RV_Register regs[RV_GP_REGISTERS_COUNT];
    RV_Register reg_pc;
    privileged_mem_t* memory;

    int (*mem_load)(struct _RV_Env* this, void *dest, RV_Ptr src, uint8_t bytes);
    int (*mem_store)(struct _RV_Env* this, RV_Ptr dest, void *src, uint8_t bytes);
} RV_Env;

extern int RV_Init(RV_Env* env, privileged_mem_t* memory);
extern RV_Env* RV_Create(privileged_mem_t* mem);
extern void RV_Exec(RV_Env *vm);
