// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#include "nbajh/buffer.h"
#include "nbajh/context.h"
#include "nbajh/errors.h"
#include "nbajh/lexer.h"
#include "nbajh/token.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Transforms a 2-byte opcode into a big-endian list of bytes.
#define NBAJH_OP2_TO_BYTES(op2) (((op2) >> 8) & 0xFF), ((op2) &0xFF)

// Transforms a dword into a little-endian list of bytes.
#define NBAJH_DWORD_TO_BYTES(dword)                                      \
    ((dword) &0xFF), (((dword) >> 8) & 0xFF), (((dword) >> 16) & 0xFF), \
        (((dword) >> 24) & 0xFF)

// Transforms a qword into a little-endian list of bytes.
#define NBAJH_QWORD_TO_BYTES(dword)                                      \
    ((dword) &0xFF), (((dword) >> 8) & 0xFF), (((dword) >> 16) & 0xFF), \
        (((dword) >> 24) & 0xFF), (((dword) >> 32) & 0xFF),             \
        (((dword) >> 40) & 0xFF), (((dword) >> 48) & 0xFF),             \
        (((dword) >> 56) & 0xFF)

// REX prefixes.
enum {
    NBAJH_REX_W = 0x48,
    NBAJH_REX_R = 0x44,
    NBAJH_REX_B = 0x41,
};

// Instruction primary opcodes.
enum {
    NBAJH_OP_ADD_RM8_IMM8 = 0x80 /* /0 ib */,
    NBAJH_OP_ADD_RM64_IMM32 = /* REX.W */ 0x81 /* /0 id */,
    NBAJH_OP_CALL_REL32 = 0xE8 /* cd */,
    NBAJH_OP_CALL_RM64 = 0xFF /* /2 */,
    NBAJH_OP_CMP_EAX_IMM32 = 0x3D /* id */,
    NBAJH_OP_CMP_RAX_IMM32 = /* REX.W */ 0x3D /* id */,
    NBAJH_OP_CMP_RM8_IMM8 = 0x80 /* /7 ib */,
    NBAJH_OP_JMP_REL8 = 0xEB /* cb */,
    NBAJH_OP_JMP_RM64 = 0xFF /* /4 */,
    NBAJH_OP_LEA_R64_M = /* REX.W */ 0x8D /* /r */,
    NBAJH_OP_MOV_RM8_R8 = 0x88 /* /r */,
    NBAJH_OP_MOV_RM64_R64 = /* REX.W */ 0x89 /* /r */,
    NBAJH_OP_MOV_R64_RM64 = /* REX.W */ 0x8B /* /r */,
    NBAJH_OP_MOV_R32_IMM32 = 0xB8 /* +rd id */,
    NBAJH_OP_MOV_R64_IMM64 = /* REX.W */ 0xB8 /* +rq iq */,
    NBAJH_OP_POP_R64 = 0x58 /* +rq */,
    NBAJH_OP_POP_RM64 = 0x8F /* /0 */,
    NBAJH_OP_PUSH_R64 = 0x50 /* +rq */,
    NBAJH_OP_RET_NEAR = 0xC3,
    NBAJH_OP_SUB_RM8_IMM8 = 0x80 /* /5 ib */,
    NBAJH_OP_SUB_RM64_IMM32 = /* REX.W */ 0x81 /* /5 id */,
    NBAJH_OP_SUB_RM64_R64 = /* REX.W */ 0x29 /* /r */,
    NBAJH_OP_XOR_RM32_R32 = 0x31 /* /r */,

    NBAJH_OP2_JAE_REL32 = 0x0F83 /* cd */,
    NBAJH_OP2_JB_REL32 = 0x0F82 /* cd */,
    NBAJH_OP2_JE_REL32 = 0x0F84 /* cd */,
    NBAJH_OP2_JNE_REL8 = 0x0F75 /* cb */,
    NBAJH_OP2_JNE_REL32 = 0x0F85 /* cd */,
    NBAJH_OP2_MOVZX_R32_RM8 = 0x0FB6 /* /r */,
};

// Register IDs.
enum {
    NBAJH_REG_AL = 0x0,

    NBAJH_REG_EAX = 0x0,
    NBAJH_REG_EDI = 0x7,

    NBAJH_REG_RAX = 0x0,
    NBAJH_REG_RBX = 0x3,
    NBAJH_REG_RSI = 0x6,
    NBAJH_REG_RDI = 0x7,
    NBAJH_REG_R12 = 0x4,
    NBAJH_REG_R13 = 0x5,
    NBAJH_REG_R14 = 0x6,
    NBAJH_REG_R15 = 0x7,
};

// ModR/M byte `mod` field values.
enum {
    NBAJH_MODRM_MOD_DISP0 = 0x0 << 6,
    NBAJH_MODRM_MOD_DISP8 = 0x1 << 6,
    NBAJH_MODRM_MOD_DIRECT = 0x3 << 6,
};

// ModR/M byte `reg` field values.
enum {
    NBAJH_MODRM_REG_OP_ADD_RM_IMM = 0x0 << 3,
    NBAJH_MODRM_REG_OP_CALL_RM = 0x2 << 3,
    NBAJH_MODRM_REG_OP_CMP_RM_IMM = 0x7 << 3,
    NBAJH_MODRM_REG_OP_SUB_RM_IMM = 0x5 << 3,
    NBAJH_MODRM_REG_OP_JMP_RM = 0x4 << 3,
    NBAJH_MODRM_REG_OP_POP_RM = 0x0 << 3,

    NBAJH_MODRM_REG_AL = NBAJH_REG_AL << 3,

    NBAJH_MODRM_REG_EAX = NBAJH_REG_EAX << 3,
    NBAJH_MODRM_REG_EDI = NBAJH_REG_EDI << 3,

    NBAJH_MODRM_REG_RBX = NBAJH_REG_RBX << 3,
    NBAJH_MODRM_REG_RSI = NBAJH_REG_RSI << 3,
    NBAJH_MODRM_REG_RDI = NBAJH_REG_RDI << 3,
    NBAJH_MODRM_REG_R14 = NBAJH_REG_R14 << 3,
    NBAJH_MODRM_REG_R15 = NBAJH_REG_R15 << 3,
};

// ModR/M byte `rm` field values.
enum {
    NBAJH_MODRM_RM_EAX = NBAJH_REG_EAX,
    NBAJH_MODRM_RM_RAX = NBAJH_REG_RAX,
    NBAJH_MODRM_RM_RBX = NBAJH_REG_RBX,
    NBAJH_MODRM_RM_RDI = NBAJH_REG_RDI,
    NBAJH_MODRM_RM_R12 = NBAJH_REG_R12,
    NBAJH_MODRM_RM_R13 = NBAJH_REG_R13,
    NBAJH_MODRM_RM_R14 = NBAJH_REG_R14,
};

enum nbajh_jit_x86_64_jump_target {
    NBAJH_JUMP_RET_ERROR_DP_OUT_OF_BOUNDS,
    NBAJH_JUMP_RET_ERROR_IO,
    NBAJH_JUMP_HANDLE_FGETC_EOF,
    NBAJH_JUMP_CALL_DEBUG_HANDLER,

    NBAJH_NUM_JUMP_TARGETS,
};

struct nbajh_jit_x86_64_jump {
    size_t from;
    enum nbajh_jit_x86_64_jump_target to;
};

struct nbajh_jit_x86_64_compiler {
    struct nbajh_lexer lexer;
    struct nbajh_token token;
    struct nbajh_buffer jumps;
    struct nbajh_buffer loop_stack;
    struct nbajh_buffer *dst;
};

static enum nbajh_error nbajh_jit_x86_64_compiler_init(
    struct nbajh_jit_x86_64_compiler *compiler,
    FILE *src,
    struct nbajh_buffer *dst
) {
    compiler->dst = dst;
    nbajh_lexer_init(&compiler->lexer, src);
    compiler->token = nbajh_lexer_next_token(&compiler->lexer);
    enum nbajh_error error = nbajh_buffer_init(&compiler->jumps);

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    return nbajh_buffer_init(&compiler->loop_stack);
}

static void
nbajh_jit_x86_64_compiler_fini(struct nbajh_jit_x86_64_compiler *compiler) {
    nbajh_buffer_fini(&compiler->jumps);
    nbajh_buffer_fini(&compiler->loop_stack);
}

static enum nbajh_error nbajh_jit_x86_64_emit_header(struct nbajh_buffer *dst) {
    uint8_t instrs[] = {
        // push rbx
        NBAJH_OP_PUSH_R64 + NBAJH_REG_RBX,
        // push r12
        NBAJH_REX_B,
        NBAJH_OP_PUSH_R64 + NBAJH_REG_R12,
        // push r13
        NBAJH_REX_B,
        NBAJH_OP_PUSH_R64 + NBAJH_REG_R13,
        // push r14
        NBAJH_REX_B,
        NBAJH_OP_PUSH_R64 + NBAJH_REG_R14,
        // push r15
        NBAJH_REX_B,
        NBAJH_OP_PUSH_R64 + NBAJH_REG_R15,
        // mov rbx, rdi
        NBAJH_REX_W,
        NBAJH_OP_MOV_RM64_R64,
        NBAJH_MODRM_MOD_DIRECT | NBAJH_MODRM_REG_RDI | NBAJH_MODRM_RM_RBX,
        // mov r12, fgetc
        NBAJH_REX_W | NBAJH_REX_B,
        NBAJH_OP_MOV_R64_IMM64 + NBAJH_REG_R12,
        NBAJH_QWORD_TO_BYTES((int64_t) fgetc),
        // mov r13, fputc
        NBAJH_REX_W | NBAJH_REX_B,
        NBAJH_OP_MOV_R64_IMM64 + NBAJH_REG_R13,
        NBAJH_QWORD_TO_BYTES((int64_t) fputc),
        // mov r14, QWORD PTR [rdi + offsetof(struct nbajh_context, dp)]
        NBAJH_REX_W | NBAJH_REX_R,
        NBAJH_OP_MOV_R64_RM64,
        NBAJH_MODRM_MOD_DISP8 | NBAJH_MODRM_REG_R14 | NBAJH_MODRM_RM_RDI,
        offsetof(struct nbajh_context, dp),
        // lea r15, QWORD PTR [rdi + offsetof(struct nbajh_context, memory)]
        NBAJH_REX_W | NBAJH_REX_R,
        NBAJH_OP_LEA_R64_M,
        NBAJH_MODRM_MOD_DISP8 | NBAJH_MODRM_REG_R15 | NBAJH_MODRM_RM_RDI,
        offsetof(struct nbajh_context, memory),
    };
    return NBAJH_BUFFER_WRITE_JIT(dst, instrs);
}

static enum nbajh_error
nbajh_jit_x86_64_set_rel8(struct nbajh_buffer *dst, size_t from, size_t to) {
    ssize_t rel8 = (ssize_t) (to - from);

    if (rel8 < INT8_MIN || rel8 > INT8_MAX) {
        return NBAJH_ERROR_JIT_JUMP_TOO_LARGE;
    }

    *(int8_t *) &dst->data[from - sizeof(int8_t)] = (int8_t) rel8;
    return NBAJH_ERROR_SUCCESS;
}

static enum nbajh_error
nbajh_jit_x86_64_set_rel32(struct nbajh_buffer *dst, size_t from, size_t to) {
    ssize_t rel32 = (ssize_t) (to - from);

    if (rel32 < INT32_MIN || rel32 > INT32_MAX) {
        return NBAJH_ERROR_JIT_JUMP_TOO_LARGE;
    }

    uint8_t *rel32_dst = &dst->data[from - sizeof(int32_t)];
    rel32_dst[0] = rel32 & 0xFF;
    rel32_dst[1] = (rel32 >> 8) & 0xFF;
    rel32_dst[2] = (rel32 >> 16) & 0xFF;
    rel32_dst[3] = (rel32 >> 24) & 0xFF;
    return NBAJH_ERROR_SUCCESS;
}

static enum nbajh_error nbajh_jit_x86_64_emit_ret_error_dp_out_of_bounds(
    size_t exit_offset,
    struct nbajh_buffer *dst
) {
    uint8_t instrs[] = {
        // mov eax, NBAJH_ERROR_DP_OUT_OF_BOUNDS
        NBAJH_OP_MOV_R32_IMM32 + NBAJH_REG_EAX,
        NBAJH_DWORD_TO_BYTES(NBAJH_ERROR_DP_OUT_OF_BOUNDS),
        // jmp .exit ; Offset written later.
        NBAJH_OP_JMP_REL8,
        0,
    };
    enum nbajh_error error = NBAJH_BUFFER_WRITE_JIT(dst, instrs);

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    return nbajh_jit_x86_64_set_rel8(dst, dst->size, exit_offset);
}

static enum nbajh_error
nbajh_jit_x86_64_emit_ret_error_io(size_t exit_offset, struct nbajh_buffer *dst) {
    uint8_t instrs[] = {
        // mov eax, NBAJH_ERROR_IO
        NBAJH_OP_MOV_R32_IMM32 + NBAJH_REG_EAX,
        NBAJH_DWORD_TO_BYTES(NBAJH_ERROR_IO),
        // jmp .exit ; Offset written later.
        NBAJH_OP_JMP_REL8,
        0,
    };
    enum nbajh_error error = NBAJH_BUFFER_WRITE_JIT(dst, instrs);

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    return nbajh_jit_x86_64_set_rel8(dst, dst->size, exit_offset);
}

static enum nbajh_error nbajh_jit_x86_64_emit_handle_fgetc_error(
    size_t exit_offset,
    struct nbajh_buffer *dst
) {
    uint8_t ret_error_io_if_ferror[] = {
        // mov rdi, QWORD PTR [rbx + offsetof(struct nbajh_context, in)]
        NBAJH_REX_W,
        NBAJH_OP_MOV_R64_RM64,
        NBAJH_MODRM_MOD_DISP8 | NBAJH_MODRM_REG_RDI | NBAJH_MODRM_RM_RBX,
        offsetof(struct nbajh_context, in),
        // mov rax, ferror
        NBAJH_REX_W,
        NBAJH_OP_MOV_R64_IMM64 + NBAJH_REG_EAX,
        NBAJH_QWORD_TO_BYTES((int64_t) ferror),
        // call rax
        NBAJH_OP_CALL_RM64,
        NBAJH_MODRM_MOD_DIRECT | NBAJH_MODRM_REG_OP_CALL_RM | NBAJH_MODRM_RM_RAX,
        // cmp eax, 0
        NBAJH_OP_CMP_EAX_IMM32,
        NBAJH_DWORD_TO_BYTES(0),
        // mov eax, NBAJH_ERROR_IO
        NBAJH_OP_MOV_R32_IMM32 + NBAJH_REG_EAX,
        NBAJH_DWORD_TO_BYTES(NBAJH_ERROR_IO),
        // jne .exit ; Offset written later.
        NBAJH_OP2_TO_BYTES(NBAJH_OP2_JNE_REL8),
        0,
    };
    enum nbajh_error error = NBAJH_BUFFER_WRITE_JIT(dst, ret_error_io_if_ferror);

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    error = nbajh_jit_x86_64_set_rel8(dst, dst->size, exit_offset);

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    uint8_t ret_error_runtime_end_of_input_file[] = {
        // mov eax, NBAJH_ERROR_RUNTIME_END_OF_INPUT_FILE
        NBAJH_OP_MOV_R32_IMM32 + NBAJH_REG_EAX,
        NBAJH_DWORD_TO_BYTES(NBAJH_ERROR_RUNTIME_END_OF_INPUT_FILE),
        // jmp .exit
        NBAJH_OP_JMP_REL8,
        0,
    };
    error = NBAJH_BUFFER_WRITE_JIT(dst, ret_error_runtime_end_of_input_file);

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    return nbajh_jit_x86_64_set_rel8(dst, dst->size, exit_offset);
}

static enum nbajh_error
nbajh_jit_x86_64_emit_call_debug_handler(struct nbajh_buffer *dst) {
    uint8_t instrs[] = {
        // pop QWORD PTR [rbx]
        NBAJH_REX_W,
        NBAJH_OP_POP_RM64,
        NBAJH_MODRM_MOD_DISP0 | NBAJH_MODRM_REG_OP_POP_RM | NBAJH_MODRM_RM_RBX,
        // mov QWORD PTR [rbx + offsetof(struct nbajh_context, dp)], r14
        NBAJH_REX_W | NBAJH_REX_R,
        NBAJH_OP_MOV_RM64_R64,
        NBAJH_MODRM_MOD_DISP8 | NBAJH_MODRM_REG_R14 | NBAJH_MODRM_RM_RBX,
        offsetof(struct nbajh_context, dp),
        // mov rdi, rbx
        NBAJH_REX_W,
        NBAJH_OP_MOV_RM64_R64,
        NBAJH_MODRM_MOD_DIRECT | NBAJH_MODRM_REG_RBX | NBAJH_MODRM_RM_RDI,
        // call QWORD PTR [rbx + offsetof(struct nbajh_context, debug_handler)]
        NBAJH_OP_CALL_RM64,
        NBAJH_MODRM_MOD_DISP8 | NBAJH_MODRM_REG_OP_CALL_RM | NBAJH_MODRM_RM_RBX,
        offsetof(struct nbajh_context, debug_handler),
        // mov r14, QWORD PTR [rbx + offsetof(struct nbajh_context, dp)]
        NBAJH_REX_W | NBAJH_REX_R,
        NBAJH_OP_MOV_R64_RM64,
        NBAJH_MODRM_MOD_DISP8 | NBAJH_MODRM_REG_R14 | NBAJH_MODRM_RM_RBX,
        offsetof(struct nbajh_context, dp),
        // jmp QWORD PTR [rbx]
        NBAJH_OP_JMP_RM64,
        NBAJH_MODRM_MOD_DISP0 | NBAJH_MODRM_REG_OP_JMP_RM | NBAJH_MODRM_RM_RBX,
    };
    return NBAJH_BUFFER_WRITE_JIT(dst, instrs);
}

static enum nbajh_error nbajh_jit_x86_64_emit_jump_target(
    enum nbajh_jit_x86_64_jump_target target,
    size_t exit_offset,
    struct nbajh_buffer *dst
) {
    switch (target) {
    case NBAJH_JUMP_RET_ERROR_DP_OUT_OF_BOUNDS:
        return nbajh_jit_x86_64_emit_ret_error_dp_out_of_bounds(
            exit_offset,
            dst
        );
    case NBAJH_JUMP_RET_ERROR_IO:
        return nbajh_jit_x86_64_emit_ret_error_io(exit_offset, dst);
    case NBAJH_JUMP_HANDLE_FGETC_EOF:
        return nbajh_jit_x86_64_emit_handle_fgetc_error(exit_offset, dst);
    case NBAJH_JUMP_CALL_DEBUG_HANDLER:
        return nbajh_jit_x86_64_emit_call_debug_handler(dst);
    default: return NBAJH_ERROR_COMPILER_INTERNAL;
    }
}

static enum nbajh_error nbajh_jit_x86_64_emit_footer(
    struct nbajh_buffer *jumps,
    struct nbajh_buffer *dst
) {
    uint8_t set_error_success[] = {
        // xor eax, eax
        NBAJH_OP_XOR_RM32_R32,
        NBAJH_MODRM_MOD_DIRECT | NBAJH_MODRM_REG_EAX | NBAJH_MODRM_RM_EAX,
    };
    enum nbajh_error error = NBAJH_BUFFER_WRITE_JIT(dst, set_error_success);

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    size_t exit_offset = dst->size;
    uint8_t exit[] = {
        // pop r15
        NBAJH_REX_B,
        NBAJH_OP_POP_R64 + NBAJH_REG_R15,
        // pop r14
        NBAJH_REX_B,
        NBAJH_OP_POP_R64 + NBAJH_REG_R14,
        // pop r13
        NBAJH_REX_B,
        NBAJH_OP_POP_R64 + NBAJH_REG_R13,
        // pop r12
        NBAJH_REX_B,
        NBAJH_OP_POP_R64 + NBAJH_REG_R12,
        // pop rbx
        NBAJH_OP_POP_R64 + NBAJH_REG_RBX,
        // ret
        NBAJH_OP_RET_NEAR,
    };
    error = NBAJH_BUFFER_WRITE_JIT(dst, exit);

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    size_t jump_target_offsets[NBAJH_NUM_JUMP_TARGETS] = { 0 };
    size_t jump_struct_size = sizeof(struct nbajh_jit_x86_64_jump);

    for (size_t i = 0; i < jumps->size; i += jump_struct_size) {
        struct nbajh_jit_x86_64_jump *jump =
            (struct nbajh_jit_x86_64_jump *) &jumps->data[i];

        if (jump_target_offsets[jump->to] == 0) {
            jump_target_offsets[jump->to] = dst->size;
            nbajh_jit_x86_64_emit_jump_target(jump->to, exit_offset, dst);
        }

        error = nbajh_jit_x86_64_set_rel32(
            dst,
            jump->from,
            jump_target_offsets[jump->to]
        );

        if (error != NBAJH_ERROR_SUCCESS) {
            return error;
        }
    }

    return NBAJH_ERROR_SUCCESS;
}

static enum nbajh_error
nbajh_jit_x86_64_emit_additive_p(struct nbajh_jit_x86_64_compiler *compiler) {
    struct nbajh_lexer *lexer = &compiler->lexer;
    enum nbajh_token_type token_type = compiler->token.type;

    uint16_t operand = 1;
    struct nbajh_token next_token;

    while ((next_token = nbajh_lexer_next_token(lexer)).type == token_type) {
        if (operand == UINT16_MAX) {
            compiler->token = next_token;
            return NBAJH_ERROR_DP_OUT_OF_BOUNDS;
        }

        operand++;
    }

    uint16_t jcc_op;
    uint8_t op;
    uint8_t modrm_reg;
    int32_t cmp_bound;

    switch (token_type) {
    case NBAJH_TOKEN_DARLING:
        jcc_op = NBAJH_OP2_JAE_REL32;
        op = NBAJH_OP_ADD_RM64_IMM32;
        modrm_reg = NBAJH_MODRM_REG_OP_ADD_RM_IMM;
        cmp_bound = NBAJH_CONTEXT_MEMORY_SIZE - operand;
        break;
    case NBAJH_TOKEN_HOLD:
        jcc_op = NBAJH_OP2_JB_REL32;
        op = NBAJH_OP_SUB_RM64_IMM32;
        modrm_reg = NBAJH_MODRM_REG_OP_SUB_RM_IMM;
        cmp_bound = operand;
        break;
    default: return NBAJH_ERROR_COMPILER_INTERNAL;
    }

    compiler->token = next_token;

    uint8_t bounds_check[] = {
        // mov rax, r14
        NBAJH_REX_W | NBAJH_REX_R,
        NBAJH_OP_MOV_RM64_R64,
        NBAJH_MODRM_MOD_DIRECT | NBAJH_MODRM_REG_R14 | NBAJH_MODRM_RM_RAX,
        // sub rax, r15
        NBAJH_REX_W | NBAJH_REX_R,
        NBAJH_OP_SUB_RM64_R64,
        NBAJH_MODRM_MOD_DIRECT | NBAJH_MODRM_REG_R15 | NBAJH_MODRM_RM_RAX,
        // cmp rax, cmp_bound
        NBAJH_REX_W,
        NBAJH_OP_CMP_RAX_IMM32,
        NBAJH_DWORD_TO_BYTES(cmp_bound),
        // (jae|jb) .ret_dp_out_of_bounds ; Offset written later.
        NBAJH_OP2_TO_BYTES(jcc_op),
        NBAJH_DWORD_TO_BYTES(0),
    };
    enum nbajh_error error = NBAJH_BUFFER_WRITE_JIT(compiler->dst, bounds_check);

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    struct nbajh_jit_x86_64_jump jump_on_error = {
        .from = compiler->dst->size,
        .to = NBAJH_JUMP_RET_ERROR_DP_OUT_OF_BOUNDS,
    };
    error = nbajh_buffer_write(
        &compiler->jumps,
        &jump_on_error,
        sizeof(jump_on_error)
    );

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    uint8_t instr[] = {
        // (add|sub) r14, operand
        NBAJH_REX_W | NBAJH_REX_B,
        op,
        NBAJH_MODRM_MOD_DIRECT | modrm_reg | NBAJH_MODRM_RM_R14,
        NBAJH_DWORD_TO_BYTES(operand),
    };
    return NBAJH_BUFFER_WRITE_JIT(compiler->dst, instr);
}

static enum nbajh_error
nbajh_jit_x86_64_emit_additive_v(struct nbajh_jit_x86_64_compiler *compiler) {
    enum nbajh_token_type token_type = compiler->token.type;
    struct nbajh_lexer *lexer = &compiler->lexer;

    uint8_t op;
    uint8_t modrm_reg;

    switch (token_type) {
    case NBAJH_TOKEN_MY:
        op = NBAJH_OP_ADD_RM8_IMM8;
        modrm_reg = NBAJH_MODRM_REG_OP_ADD_RM_IMM;
        break;
    case NBAJH_TOKEN_HAND:
        op = NBAJH_OP_SUB_RM8_IMM8;
        modrm_reg = NBAJH_MODRM_REG_OP_SUB_RM_IMM;
        break;
    default: return NBAJH_ERROR_COMPILER_INTERNAL;
    }

    uint8_t operand = 1;
    struct nbajh_token next_token;

    while ((next_token = nbajh_lexer_next_token(lexer)).type == token_type) {
        operand++;
    }

    compiler->token = next_token;

    if (operand == 0) {
        return NBAJH_ERROR_SUCCESS;
    }

    uint8_t instr[] = {
        // (add|sub) BYTE PTR [r14], operand
        NBAJH_REX_B,
        op,
        NBAJH_MODRM_MOD_DISP0 | modrm_reg | NBAJH_MODRM_RM_R14,
        operand,
    };
    return NBAJH_BUFFER_WRITE_JIT(compiler->dst, instr);
}

static enum nbajh_error
nbajh_jit_x86_64_emit_write(struct nbajh_jit_x86_64_compiler *compiler) {
    uint8_t instrs[] = {
        // movzx edi, BYTE PTR [r14]
        NBAJH_REX_B,
        NBAJH_OP2_TO_BYTES(NBAJH_OP2_MOVZX_R32_RM8),
        NBAJH_MODRM_MOD_DISP0 | NBAJH_MODRM_REG_EDI | NBAJH_MODRM_RM_R14,
        // mov rsi, QWORD PTR [rbx + offsetof(struct nbajh_context, out)]
        NBAJH_REX_W,
        NBAJH_OP_MOV_R64_RM64,
        NBAJH_MODRM_MOD_DISP8 | NBAJH_MODRM_REG_RSI | NBAJH_MODRM_RM_RBX,
        offsetof(struct nbajh_context, out),
        // call r13
        NBAJH_REX_B,
        NBAJH_OP_CALL_RM64,
        NBAJH_MODRM_MOD_DIRECT | NBAJH_MODRM_REG_OP_CALL_RM | NBAJH_MODRM_RM_R13,
        // cmp eax, -1
        NBAJH_OP_CMP_EAX_IMM32,
        NBAJH_DWORD_TO_BYTES((int32_t) -1),
        // je .ret_error_io ; Offset written later.
        NBAJH_OP2_TO_BYTES(NBAJH_OP2_JE_REL32),
        NBAJH_DWORD_TO_BYTES(0),
    };
    enum nbajh_error error = NBAJH_BUFFER_WRITE_JIT(compiler->dst, instrs);

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    struct nbajh_jit_x86_64_jump jump = {
        .from = compiler->dst->size,
        .to = NBAJH_JUMP_RET_ERROR_IO,
    };
    return nbajh_buffer_write(&compiler->jumps, &jump, sizeof(jump));
}

static enum nbajh_error
nbajh_jit_x86_64_emit_read(struct nbajh_jit_x86_64_compiler *compiler) {
    uint8_t call_fgetc[] = {
        // mov rdi, QWORD PTR [rbx + offsetof(struct nbajh_context, in)]
        NBAJH_REX_W,
        NBAJH_OP_MOV_R64_RM64,
        NBAJH_MODRM_MOD_DISP8 | NBAJH_MODRM_REG_RDI | NBAJH_MODRM_RM_RBX,
        offsetof(struct nbajh_context, in),
        // call r12
        NBAJH_REX_B,
        NBAJH_OP_CALL_RM64,
        NBAJH_MODRM_MOD_DIRECT | NBAJH_MODRM_REG_OP_CALL_RM | NBAJH_MODRM_RM_R12,
        // cmp eax, -1
        NBAJH_OP_CMP_EAX_IMM32,
        NBAJH_DWORD_TO_BYTES((int32_t) -1),
        // je .handle_fgetc_eof ; Offset written later.
        NBAJH_OP2_TO_BYTES(NBAJH_OP2_JE_REL32),
        NBAJH_DWORD_TO_BYTES(0),
    };
    enum nbajh_error error = NBAJH_BUFFER_WRITE_JIT(compiler->dst, call_fgetc);

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    struct nbajh_jit_x86_64_jump jump = {
        .from = compiler->dst->size,
        .to = NBAJH_JUMP_HANDLE_FGETC_EOF,
    };
    error = nbajh_buffer_write(&compiler->jumps, &jump, sizeof(jump));

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    uint8_t write_returned_char_to_memory[] = {
        // mov BYTE PTR [r14], al
        NBAJH_REX_B,
        NBAJH_OP_MOV_RM8_R8,
        NBAJH_MODRM_MOD_DISP0 | NBAJH_MODRM_REG_AL | NBAJH_MODRM_RM_R14,
    };
    return NBAJH_BUFFER_WRITE_JIT(compiler->dst, write_returned_char_to_memory);
}

static enum nbajh_error
nbajh_jit_x86_64_begin_loop(struct nbajh_jit_x86_64_compiler *compiler) {
    uint8_t instrs[] = {
        // cmp BYTE PTR [r14], 0
        NBAJH_REX_B,
        NBAJH_OP_CMP_RM8_IMM8,
        NBAJH_MODRM_MOD_DISP0 | NBAJH_MODRM_REG_OP_CMP_RM_IMM | NBAJH_MODRM_RM_R14,
        0,
        // je loop_end ; Offset written later.
        NBAJH_OP2_TO_BYTES(NBAJH_OP2_JE_REL32),
        NBAJH_DWORD_TO_BYTES(0),
    };
    enum nbajh_error error = NBAJH_BUFFER_WRITE_JIT(compiler->dst, instrs);

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    return nbajh_buffer_write_size(&compiler->loop_stack, compiler->dst->size);
}

static enum nbajh_error
nbajh_jit_x86_64_end_loop(struct nbajh_jit_x86_64_compiler *compiler) {
    if (compiler->loop_stack.size == 0) {
        return NBAJH_ERROR_COMPILER_UNEXPECTED_LOOP_END;
    }

    uint8_t instrs[] = {
        // cmp BYTE PTR [r14], 0
        NBAJH_REX_B,
        NBAJH_OP_CMP_RM8_IMM8,
        NBAJH_MODRM_MOD_DISP0 | NBAJH_MODRM_REG_OP_CMP_RM_IMM | NBAJH_MODRM_RM_R14,
        0,
        // jne loop_start ; Offset written later.
        NBAJH_OP2_TO_BYTES(NBAJH_OP2_JNE_REL32),
        NBAJH_DWORD_TO_BYTES(0),
    };
    enum nbajh_error error = NBAJH_BUFFER_WRITE_JIT(compiler->dst, instrs);

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    size_t loop_start = nbajh_buffer_pop_size(&compiler->loop_stack);
    error = nbajh_jit_x86_64_set_rel32(
        compiler->dst,
        compiler->dst->size,
        loop_start
    );

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    return nbajh_jit_x86_64_set_rel32(
        compiler->dst,
        loop_start,
        compiler->dst->size
    );
}

static enum nbajh_error
nbajh_jit_x86_64_emit_debug(struct nbajh_jit_x86_64_compiler *compiler) {
    uint8_t instr[] = {
        // call .call_debug_handler ; Offset written later.
        NBAJH_OP_CALL_REL32,
        NBAJH_DWORD_TO_BYTES(0),
    };
    enum nbajh_error error = NBAJH_BUFFER_WRITE_JIT(compiler->dst, instr);

    if (error != NBAJH_ERROR_SUCCESS) {
        return error;
    }

    struct nbajh_jit_x86_64_jump jump = {
        .from = compiler->dst->size,
        .to = NBAJH_JUMP_CALL_DEBUG_HANDLER,
    };
    return nbajh_buffer_write(&compiler->jumps, &jump, sizeof(jump));
}

static enum nbajh_error
nbajh_jit_x86_64_emit(struct nbajh_jit_x86_64_compiler *compiler) {
    enum nbajh_error error;

    switch (compiler->token.type) {
    case NBAJH_TOKEN_DARLING:
    case NBAJH_TOKEN_HOLD: return nbajh_jit_x86_64_emit_additive_p(compiler);
    case NBAJH_TOKEN_MY:
    case NBAJH_TOKEN_HAND: return nbajh_jit_x86_64_emit_additive_v(compiler);
    case NBAJH_TOKEN_NOTHING: error = nbajh_jit_x86_64_emit_write(compiler); break;
    case NBAJH_TOKEN_BEATS: error = nbajh_jit_x86_64_emit_read(compiler); break;
    case NBAJH_TOKEN_A: error = nbajh_jit_x86_64_begin_loop(compiler); break;
    case NBAJH_TOKEN_JET2: error = nbajh_jit_x86_64_end_loop(compiler); break;
    case NBAJH_TOKEN_HOLIDAY: error = nbajh_jit_x86_64_emit_debug(compiler); break;
    default: return NBAJH_ERROR_COMPILER_INVALID_TOKEN;
    }

    if (error == NBAJH_ERROR_SUCCESS) {
        compiler->token = nbajh_lexer_next_token(&compiler->lexer);
    }

    return error;
}

enum nbajh_error nbajh_compile_jit_x86_64(
    FILE *src,
    struct nbajh_buffer *dst,
    struct nbajh_token *last_token_dst
) {
    struct nbajh_jit_x86_64_compiler compiler;
    enum nbajh_error error = nbajh_jit_x86_64_compiler_init(&compiler, src, dst);

    if (error != NBAJH_ERROR_SUCCESS) {
        *last_token_dst = compiler.token;
        return error;
    }

    error = nbajh_jit_x86_64_emit_header(dst);

    if (error != NBAJH_ERROR_SUCCESS) {
        *last_token_dst = compiler.token;
        nbajh_jit_x86_64_compiler_fini(&compiler);
        return error;
    }

    while (compiler.token.type != NBAJH_TOKEN_EOF) {
        error = nbajh_jit_x86_64_emit(&compiler);

        if (error != NBAJH_ERROR_SUCCESS) {
            *last_token_dst = compiler.token;
            nbajh_jit_x86_64_compiler_fini(&compiler);
            return error;
        }
    }

    *last_token_dst = compiler.token;

    if (compiler.loop_stack.size != 0) {
        nbajh_jit_x86_64_compiler_fini(&compiler);
        return NBAJH_ERROR_COMPILER_UNCLOSED_LOOPS;
    }

    error = nbajh_jit_x86_64_emit_footer(&compiler.jumps, dst);
    nbajh_jit_x86_64_compiler_fini(&compiler);
    return error;
}
