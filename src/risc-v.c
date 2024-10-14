#include "risc-v.h"
#include "check_config.h"

#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#include "error.h"

#define JMP_FLAG_SET() flags |= (RV_Uint)0b1
#define JMP_FLAG_CLR() flags &= !(RV_Uint)0b1
#define JMP_FLAG_GET() flags &(RV_Uint)0b1
static RV_Uint flags = 0;

// todo: add support for rv64i
static RV_Int sign_extend(RV_Uint value, uint8_t src_bits, uint8_t dest_bits)
{
    assert(src_bits <= dest_bits);
    assert(src_bits > 0);
    if (__glibc_unlikely(src_bits >= dest_bits))
    {
        return (RV_Int)value;
    }
    RV_Uint sign_bit = value & (1ull << (src_bits - 1));
    if (!sign_bit)
    {
        return (RV_Int)value;
    }
    RV_Uint extension_mask = ((1 << (dest_bits - src_bits)) - 1) << src_bits;
    return value | extension_mask;
}

#define RV_GET_OPCODE(cmd) ((RV_Uint)((cmd) & 0b1111111))
#define RV_GET_RD(cmd) ((RV_Register)((cmd >> 7) & 0b11111))
#define RV_GET_RS1(cmd) ((RV_Register)((cmd >> 15) & 0b11111))
#define RV_GET_RS2(cmd) ((RV_Register)((cmd >> 20) & 0b11111))
#define RV_GET_FUNCT3(cmd) ((RV_Uint)((cmd >> 12) & 0b111))
#define RV_GET_FUNCT7(cmd) ((RV_Uint)((cmd >> 25) & 0b1111111))
#define RV_FUNCT7_SET 0b0100000
#define RV_FUNCT7_CLR 0b0000000

static void lui(RV_Env *vm, RV_Cmd cmd)
{
    RV_Uint imm = cmd & (~0b111111111111u);
    vm->regs[RV_GET_RD(cmd)] = imm;
}
static void auipc(RV_Env *vm, RV_Cmd cmd)
{
    RV_Uint imm = cmd & (~0b111111111111u);
    vm->regs[RV_GET_RD(cmd)] = imm + vm->reg_pc;
}

static void jal(RV_Env *vm, RV_Cmd cmd)
{
    RV_RegisterIndex rd = RV_GET_RD(cmd);
    if (rd != 0)
    {
        vm->regs[rd] = vm->reg_pc + 4;
    }
    RV_Uint offset10_1 = (cmd >> 20) & 0b11111111110;
    RV_Uint offset11 = (cmd >> (20 - 11)) & (1u << 11);
    RV_Uint offset19_12 = cmd & (0b11111111u << 12);
    RV_Uint offset_sign_bit = (cmd >> (31 - 20)) & (1ull << 20);
    RV_Uint offset_bits = offset_sign_bit | offset19_12 | offset11 | offset10_1 | 0b0u;
    RV_Int offset = sign_extend(offset_bits, 20, sizeof(RV_Int) * 8);
    vm->reg_pc += offset;
    JMP_FLAG_SET();
}
static void jalr(RV_Env *vm, RV_Cmd cmd)
{
    uint8_t funct3 = RV_GET_FUNCT3(cmd);
    if (funct3 != 0b000)
    {
        error_handler();
        return;
    }
    RV_RegisterIndex rd = RV_GET_RD(cmd);
    RV_RegisterIndex rs1 = RV_GET_RS1(cmd);
    if (rd != 0)
    {
        vm->regs[rd] = vm->reg_pc + 4;
    }
    RV_Uint offset_bits = (cmd >> 20);
    RV_Int offset = sign_extend(offset_bits, 12, sizeof(RV_Int) * 8);
    RV_Ptr addr = (vm->regs[rs1] + offset) & (~0b1u);
    vm->reg_pc = addr;
    JMP_FLAG_SET();
}

#define RV32I_BRANCH_BEQ 0b000
#define RV32I_BRANCH_BNE 0b001
#define RV32I_BRANCH_BLT 0b100
#define RV32I_BRANCH_BGE 0b101
#define RV32I_BRANCH_BLTU 0b110
#define RV32I_BRANCH_BGEU 0b111
static void branch(RV_Env *vm, RV_Cmd cmd)
{
    RV_RegisterIndex rs1 = RV_GET_RS1(cmd);
    RV_RegisterIndex rs2 = RV_GET_RS2(cmd);
    RV_Int diff = ((RV_Int *)vm->regs)[rs1] - ((RV_Int *)vm->regs)[rs2];
    RV_Int udiff = vm->regs[rs1] - vm->regs[rs2];

    // RV_Uint offset_sign_bit = (cmd >> (13 - 12)) & (1u << 12);
    // RV_Uint offset10_5 = ((cmd >> 25) & 0b111111) << 5;
    // RV_Uint offset4_1 = ((cmd >> 8) & 0b1111);
    // RV_Uint offset11 = ((cmd >> 7) & 1) << 11;
    // RV_Uint offset_bits = offset_sign_bit | offset11 | offset10_5 | offset4_1;

    RV_Uint offset_sign_bit = (cmd >> (31 - 12)) & (1ull<<12);
    RV_Uint offset10_5 = (cmd >> (25 - 5)) & (0b111111ull<<5);
    RV_Uint offset4_1 = (cmd >> (8 - 1)) & (0b1111ull<<1);
    RV_Uint offset11 = (cmd << (11 - 7)) & (0b1ull<<11);
    RV_Uint offset_bits = offset_sign_bit | offset11 | offset10_5 | offset4_1;

    RV_Int offset = sign_extend(offset_bits, 12, sizeof(RV_Int) * 8);
    switch (RV_GET_FUNCT3(cmd))
    {
    case RV32I_BRANCH_BEQ:
        if (udiff == 0)
            break;
        goto ret;
    case RV32I_BRANCH_BNE:
        if (udiff != 0)
            break;
        goto ret;
    case RV32I_BRANCH_BLT:
        if (diff < 0)
            break;
        goto ret;
    case RV32I_BRANCH_BLTU:
        if (udiff < 0)
            break;
        goto ret;
    case RV32I_BRANCH_BGE:
        if (diff >= 0)
            break;
        goto ret;
    case RV32I_BRANCH_BGEU:
        if (udiff >= 0)
            break;
        goto ret;
    default:
        error_handler();
        goto ret;
    }
    vm->reg_pc += offset;
    JMP_FLAG_SET();
    ret:
}

static void load(RV_Env *vm, RV_Cmd cmd)
{
    RV_RegisterIndex rs1 = RV_GET_RS1(cmd);
    RV_RegisterIndex rd = RV_GET_RD(cmd);

    RV_Uint offset_bits = cmd >> 20;
    RV_Int offset = sign_extend(offset_bits, 12, sizeof(RV_Int) * 8);
    RV_Uint base = vm->regs[rs1];
    uint8_t load_width_in_bytes;
    uint8_t use_sign_extension = 0;
    uint8_t funct3 = RV_GET_FUNCT3(cmd);
    switch (funct3)
    {
    case 0b000: // lb
        load_width_in_bytes = 1;
        use_sign_extension = 1;
        break;
    case 0b001: // lh
        load_width_in_bytes = 2;
        use_sign_extension = 1;
        break;
    case 0b010: // lw
        load_width_in_bytes = 4;
        use_sign_extension = 1;
        break;
    case 0b100: // lbu
        load_width_in_bytes = 1;
        break;
    case 0b101: // lhu
        load_width_in_bytes = 2;
        break;
#ifdef CONFIG_USE_RV64
    case 0b110: // lwu
        load_width_in_bytes = 4;
        break;
    case 0b011: // ld
        load_width_in_bytes = 8;
        break;
#endif
    default:
        ERROR_LOG("unknown load type detected (funct3=0x%x)\n", funct3);
        error_handler();
        return;
    }

    RV_Uint tmp = 0;
    vm->mem_load(&tmp, (RV_Ptr)(base + offset), load_width_in_bytes);

    if (use_sign_extension)
    {
        tmp = (RV_Uint)sign_extend(tmp, load_width_in_bytes * 8, sizeof(RV_Int) * 8);
    }
    vm->regs[rd] = tmp;
}
static void store(RV_Env *vm, RV_Cmd cmd)
{
    RV_Uint offset11_5 = (cmd >> (25 - 5)) & 0b111111100000;
    RV_Uint offset4_0 = (cmd >> 7) & 0b11111;
    RV_Uint offset_bits = offset11_5 | offset4_0;
    RV_Int offset = sign_extend(offset_bits, 12, sizeof(RV_Int) * 8);

    void *src = (void *)&(vm->regs[RV_GET_RS2(cmd)]);
    RV_Uint base = vm->regs[RV_GET_RS1(cmd)];
    RV_Uint width;

    switch (RV_GET_FUNCT3(cmd))
    {
    case 0b000:
        width = 1;
        break;
    case 0b001:
        width = 2;
        break;
    case 0b010:
        width = 4;
        break;
#ifdef CONFIG_USE_RV64
    case 0b011:
        width = 8;
        break;
#endif
    default:
        error_handler();
        return;
    }

    vm->mem_store((RV_Ptr)(base + offset), src, width);
}

#define OP_IMM_ADDI 0b000
#define OP_IMM_SLTI 0b010
#define OP_IMM_SLTIU 0b011
#define OP_IMM_XORI 0b100
#define OP_IMM_ORI 0b110
#define OP_IMM_ANDI 0b111
#define OP_IMM_SLLI 0b001
#define OP_IMM_SRLI_OR_SRAI 0b101
static RV_Uint op_imm_shift(RV_Env *vm, RV_Uint imm, RV_Uint src, RV_Uint funct3);
static void op_imm(RV_Env *vm, RV_Cmd cmd)
{
    RV_Uint imm_bits = (cmd >> 20);
    RV_Int imm_signed = sign_extend(imm_bits, 12, sizeof(RV_Int) * 8);
    RV_Uint imm = imm_bits;
    RV_Uint src = vm->regs[RV_GET_RS1(cmd)];
    RV_Uint ans = 0;
    RV_Uint funct3 = RV_GET_FUNCT3(cmd);
    switch (funct3)
    {
    case OP_IMM_ADDI:
        ans = (RV_Uint)(((RV_Int)src) + imm_signed);
        break;
    case OP_IMM_SLTI:
        ans = (RV_Uint)(((RV_Int)src) < imm_signed);
        break;
    case OP_IMM_SLTIU:
        ans = src < imm;
        break;
    case OP_IMM_ANDI:
        ans = src & imm_signed;
        break;
    case OP_IMM_ORI:
        ans = src | imm_signed;
        break;
    case OP_IMM_XORI:
        ans = (src) ^ (imm_signed);
        break;
    case OP_IMM_SLLI:
        ans = op_imm_shift(vm, imm, src, funct3);
        break;
    case OP_IMM_SRLI_OR_SRAI:
        ans = op_imm_shift(vm, imm, src, funct3);
        break;
    default:
        error_handler();
        return;
    }
    vm->regs[RV_GET_RD(cmd)] = ans;
}
static inline RV_Uint op_imm_shift(RV_Env *vm, RV_Uint imm, RV_Uint src, RV_Uint funct3)
{
    RV_Uint imm11_5 = imm >> 5;
    RV_Uint shamt = imm & 0b11111;
    if (funct3 == OP_IMM_SLLI && imm11_5 == 0b0000000)
    {
        return src << shamt;
    }
    else if (imm11_5 == 0b0000000)
    {
        return src >> shamt;
    }
    else if (imm11_5 == 0b0100000)
    {
        return ((RV_Int)src) >> shamt;
    }
    error_handler();
    return 0;
}

#define OP_ADD_OR_SUB 0b000
#define OP_SLL 0b001
#define OP_SLT 0b010
#define OP_SLTU 0b011
#define OP_XOR 0b100
#define OP_SRL_OR_SRA 0b101
#define OP_OR 0b110
#define OP_AND 0b111
static void op(RV_Env *vm, RV_Cmd cmd)
{
    RV_Uint funct7 = RV_GET_FUNCT7(cmd);
    RV_Uint funct3 = RV_GET_FUNCT3(cmd);
    RV_Int src1 = (RV_Int)vm->regs[RV_GET_RS1(cmd)];
    RV_Int src2 = (RV_Int)vm->regs[RV_GET_RS2(cmd)];
    RV_Int ans = 0;
    switch (funct3)
    {
    case OP_ADD_OR_SUB:
        if (funct7 == RV_FUNCT7_CLR)
        {
            ans = src1 + src2;
        }
        else if (funct7 == RV_FUNCT7_SET)
        {
            ans = src1 - src2;
        }
        else
        {
            goto error;
        }
        break;
    case OP_SLL:
        ans = (RV_Int)(((RV_Uint)src1) << ((RV_Uint)src2 & 0b11111));
        break;
    case OP_SLT:
        ans = src1 > src2;
        break;
    case OP_SLTU:
        ans = ((RV_Uint)src1) > ((RV_Uint)src2);
        break;
    case OP_XOR:
        ans = src1 ^ src2;
        break;
    case OP_SRL_OR_SRA:
        RV_Uint shamt = (RV_Uint)src2 & 0b11111;
        if (funct7 == RV_FUNCT7_CLR)
        {
            ans = (RV_Int) ~((((RV_Uint)~src1)) >> shamt);
        }
        else if (funct7 == RV_FUNCT7_SET)
        {
            ans = (RV_Int)(((RV_Uint)src1) >> shamt);
        }
        else
        {
            goto error;
        }
        break;
    case OP_OR:
        ans = src1 | src2;
        break;
    case OP_AND:
        ans = src1 & src2;
        break;
    default:
        goto error;
    }
    vm->regs[RV_GET_RD(cmd)] = ans;
    return;
error:
    error_handler();
}

/* FENCE & FENCE.TSO & PAUSE */
static void misc_mem(RV_Env *vm, RV_Cmd cmd)
{
#ifdef CONFIG_DYNAMIC_ERROR_LOG
    ERROR_LOG("error: fence type instruction detected (");
    if (cmd >> 28 == 0b1000)
    {
        ERROR_LOG("fence.tso");
    }
    else if (cmd >> 28 == 0b0000)
    {
        if (cmd == 0b00000001000000000000000000001111u)
        {
            ERROR_LOG("pause");
        }
        else
        {
            ERROR_LOG("fence");
        }
    }
    else
    {
        ERROR_LOG("0x%08x", cmd);
    }
    ERROR_LOG(")\n");
#endif // todo: tmp
    assert(0); // todo: this function depends on multi-threading
}
/* ECALL & EBREAK */
static void system(RV_Env *vm, RV_Cmd cmd)
{
#ifdef CONFIG_DYNAMIC_ERROR_LOG
    (void)fprintf(stderr, "error: system type instruction detected (");
    if (cmd == 0b1110011)
    {
        (void)fprintf(stderr, "ecall");
    }
    else if (cmd == (0b1110011 | (1ull << 20)))
    {
        (void)fprintf(stderr, "ebreak");
    }
    else
    {
        (void)fprintf(stderr, "0x%08x", cmd);
    }
    (void)fprintf(stderr, ")\n");
#endif
    assert(0); // todo: this function requires previliged architecture
}

#ifdef CONFIG_USE_RV64

#define OP_IMM_32_ADDIW 0b000
#define OP_IMM_32_SLLIW 0b001
#define OP_IMM_32_SRLIW_OR_SRAIW 0b101
static void op_imm_32(RV_Env *vm, RV_Cmd cmd)
{
    uint32_t src = (RV_Int)vm->regs[RV_GET_RS1(cmd)];
    uint32_t imm_bits = cmd >> 20u;
    uint8_t shamt = imm_bits & 0b1111;
    uint32_t result;
    switch (RV_GET_FUNCT3(cmd))
    {
    case OP_IMM_32_ADDIW:
        int32_t imm = sign_extend(imm_bits, 12, sizeof(RV_Int) * 8);
        result = (uint32_t)(src + imm);
        break;
    case OP_IMM_32_SLLIW:
        result = src << shamt;
        break;
    case OP_IMM_32_SRLIW_OR_SRAIW:
        uint8_t imm11_6 = cmd >> 26;
        if (imm11_6 == 0b000000)
        {
            result = src >> shamt;
        }
        else if (imm11_6 == 0b010000)
        {
            result = (uint32_t)(((int32_t)imm_bits) >> shamt);
        }
        else
        {
            goto error;
        }
        break;
    default:
        goto error;
    }
    vm->regs[RV_GET_RD(cmd)] = result;
    return;
error:
    error_handler();
}

#endif // CONFIG_USE_RV64

#define OPCODE_LUI 0b0110111
#define OPCODE_AUIPC 0b0010111
#define OPCODE_JAL 0b1101111
#define OPCODE_JALR 0b1100111
#define OPCODE_BRANCH 0b1100011
#define OPCODE_LOAD 0b0000011
#define OPCODE_STORE 0b0100011
#define OPCODE_OP_IMM 0b0010011
#define OPCODE_OP 0b0110011
#define OPCODE_MISC_MEM 0b0001111
#define OPCODE_SYSTEM 0b1110011
#define OPCODE_OP_IMM_32 0b0011011
static struct
{
    RV_Cmd opcode;
    void (*handler)(RV_Env *, RV_Cmd);
} RV_InstructionTable[] = {
    /* RV32I */
    {OPCODE_LUI, lui},
    {OPCODE_AUIPC, auipc},

    {OPCODE_JAL, jal},
    {OPCODE_JALR, jalr},

    {OPCODE_BRANCH, branch},

    {OPCODE_LOAD, load},
    {OPCODE_STORE, store},

    {OPCODE_OP_IMM, op_imm},
    {OPCODE_OP, op},

    {OPCODE_MISC_MEM, misc_mem},
    {OPCODE_SYSTEM, system},

#ifdef CONFIG_USE_RV64
    /* RV64I */
    {OPCODE_OP_IMM_32, op_imm_32},

#endif // CONFIG_USE_RV64
};

void RV_Exec(RV_Env *vm)
{
    RV_Cmd cmd = 0;
    vm->mem_load(&cmd, (RV_Ptr)vm->reg_pc, sizeof(RV_Cmd));
    int i = 0;
    for (; i < sizeof(RV_InstructionTable) / sizeof(*RV_InstructionTable); i++)
    {
        if (RV_GET_OPCODE(cmd) == RV_InstructionTable[i].opcode)
        {
            break;
        }
    }

    if (i >= sizeof(RV_InstructionTable) / sizeof(*RV_InstructionTable))
    {
#ifdef CONFIG_DYNAMIC_ERROR_LOG
        fprintf(stderr, "error: unknonw instruction detected (0x%08x)\n", cmd);
        assert(0);
#endif
        error_handler();//todo: tmp
    }
    else
    {
        RV_InstructionTable[i].handler(vm, cmd);
        vm->regs[0] = 0;
    }
    if (JMP_FLAG_GET())
    {
        JMP_FLAG_CLR();
    }
    else
    {
        vm->reg_pc += 4;
    }
}
