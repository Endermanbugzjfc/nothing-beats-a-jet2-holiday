// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#include "nbajh/buffer.h"
#include "nbajh/errors.h"
#include "nbajh/lexer.h"
#include "nbajh/targets/bytecode.h"
#include "nbajh/token.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DEFINE_NBAJH_BYTECODE_INSTR_TYPE_WITH_OPERAND(name, operand_type) \
    typedef uint8_t name[1 + sizeof(operand_type)];                      \
                                                                         \
    void name##_init(name instr, uint8_t op, operand_type operand) {     \
        instr[0] = op;                                                   \
        memcpy(&instr[1], &operand, sizeof(operand));                    \
    }

DEFINE_NBAJH_BYTECODE_INSTR_TYPE_WITH_OPERAND(nbajh_bytecode_instr_u8, uint8_t)
DEFINE_NBAJH_BYTECODE_INSTR_TYPE_WITH_OPERAND(nbajh_bytecode_instr_u16, uint16_t)
DEFINE_NBAJH_BYTECODE_INSTR_TYPE_WITH_OPERAND(nbajh_bytecode_instr_size, size_t)

struct nbajh_bytecode_compiler {
    struct nbajh_lexer lexer;
    struct nbajh_token token;
    struct nbajh_buffer loop_stack;
    struct nbajh_buffer *dst;
};

static enum nbajh_error nbajh_bytecode_compiler_init(
    struct nbajh_bytecode_compiler *compiler,
    FILE *src,
    struct nbajh_buffer *dst
) {
    compiler->dst = dst;
    nbajh_lexer_init(&compiler->lexer, src);
    compiler->token = nbajh_lexer_next_token(&compiler->lexer);
    return nbajh_buffer_init(&compiler->loop_stack);
}

static void nbajh_bytecode_compiler_fini(struct nbajh_bytecode_compiler *compiler
) {
    nbajh_buffer_fini(&compiler->loop_stack);
}

static enum nbajh_error
nbajh_bytecode_emit_additive_p(struct nbajh_bytecode_compiler *compiler) {
    enum nbajh_token_type token_type = compiler->token.type;
    struct nbajh_lexer *lexer = &compiler->lexer;

    uint8_t op;

    switch (token_type) {
    case NBAJH_TOKEN_DARLING: op = NBAJH_BYTECODE_OP_ADDP; break;
    case NBAJH_TOKEN_HOLD: op = NBAJH_BYTECODE_OP_SUBP; break;
    default: return NBAJH_ERROR_COMPILER_INTERNAL;
    }

    uint16_t operand = 1;
    struct nbajh_token next_token;

    while ((next_token = nbajh_lexer_next_token(lexer)).type == token_type) {
        if (operand == UINT16_MAX) {
            compiler->token = next_token;
            return NBAJH_ERROR_DP_OUT_OF_BOUNDS;
        }

        operand++;
    }

    compiler->token = next_token;

    nbajh_bytecode_instr_u16 instr;
    nbajh_bytecode_instr_u16_init(instr, op, operand);
    return NBAJH_BUFFER_WRITE(compiler->dst, instr);
}

static enum nbajh_error
nbajh_bytecode_emit_additive_v(struct nbajh_bytecode_compiler *compiler) {
    enum nbajh_token_type token_type = compiler->token.type;
    struct nbajh_lexer *lexer = &compiler->lexer;

    uint8_t op;

    switch (token_type) {
    case NBAJH_TOKEN_MY: op = NBAJH_BYTECODE_OP_ADDV; break;
    case NBAJH_TOKEN_HAND: op = NBAJH_BYTECODE_OP_SUBV; break;
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

    nbajh_bytecode_instr_u8 instr;
    nbajh_bytecode_instr_u8_init(instr, op, operand);
    return NBAJH_BUFFER_WRITE(compiler->dst, instr);
}

static enum nbajh_error
nbajh_bytecode_emit_no_operand(struct nbajh_bytecode_compiler *compiler) {
    uint8_t op;

    switch (compiler->token.type) {
    case NBAJH_TOKEN_NOTHING: op = NBAJH_BYTECODE_OP_WRITE; break;
    case NBAJH_TOKEN_BEATS: op = NBAJH_BYTECODE_OP_READ; break;
    case NBAJH_TOKEN_HOLIDAY: op = NBAJH_BYTECODE_OP_DEBUG; break;
    default: return NBAJH_ERROR_COMPILER_INTERNAL;
    }

    return nbajh_buffer_write_u8(compiler->dst, op);
}

static enum nbajh_error
nbajh_bytecode_begin_loop(struct nbajh_bytecode_compiler *compiler) {
    compiler->dst->size += sizeof(nbajh_bytecode_instr_size);
    return nbajh_buffer_write_size(&compiler->loop_stack, compiler->dst->size);
}

static enum nbajh_error
nbajh_bytecode_end_loop(struct nbajh_bytecode_compiler *compiler) {
    if (compiler->loop_stack.size == 0) {
        return NBAJH_ERROR_COMPILER_UNEXPECTED_LOOP_END;
    }

    size_t loop_start = nbajh_buffer_pop_size(&compiler->loop_stack);
    nbajh_bytecode_instr_size instr;
    nbajh_bytecode_instr_size_init(instr, NBAJH_BYTECODE_OP_JMPNZ, loop_start);
    enum nbajh_error error = NBAJH_BUFFER_WRITE(compiler->dst, instr);

    size_t loop_end = compiler->dst->size;
    nbajh_bytecode_instr_size_init(instr, NBAJH_BYTECODE_OP_JMPZ, loop_end);
    size_t jmpz_instr_offset = loop_start - sizeof(nbajh_bytecode_instr_size);
    memcpy(&compiler->dst->data[jmpz_instr_offset], instr, sizeof(instr));

    return error;
}

static enum nbajh_error
nbajh_bytecode_emit(struct nbajh_bytecode_compiler *compiler) {
    enum nbajh_error error;

    switch (compiler->token.type) {
    case NBAJH_TOKEN_DARLING:
    case NBAJH_TOKEN_HOLD: return nbajh_bytecode_emit_additive_p(compiler);
    case NBAJH_TOKEN_MY:
    case NBAJH_TOKEN_HAND: return nbajh_bytecode_emit_additive_v(compiler);
    case NBAJH_TOKEN_NOTHING:
    case NBAJH_TOKEN_BEATS:
    case NBAJH_TOKEN_HOLIDAY:
        error = nbajh_bytecode_emit_no_operand(compiler);
        break;
    case NBAJH_TOKEN_A: error = nbajh_bytecode_begin_loop(compiler); break;
    case NBAJH_TOKEN_JET2: error = nbajh_bytecode_end_loop(compiler); break;
    default: return NBAJH_ERROR_COMPILER_INVALID_TOKEN;
    }

    if (error == NBAJH_ERROR_SUCCESS) {
        compiler->token = nbajh_lexer_next_token(&compiler->lexer);
    }

    return error;
}

enum nbajh_error nbajh_compile_bytecode(
    FILE *src,
    struct nbajh_buffer *dst,
    struct nbajh_token *last_token_dst
) {
    struct nbajh_bytecode_compiler compiler;
    enum nbajh_error error = nbajh_bytecode_compiler_init(&compiler, src, dst);

    if (error != NBAJH_ERROR_SUCCESS) {
        *last_token_dst = compiler.token;
        return error;
    }

    while (compiler.token.type != NBAJH_TOKEN_EOF) {
        error = nbajh_bytecode_emit(&compiler);

        if (error != NBAJH_ERROR_SUCCESS) {
            *last_token_dst = compiler.token;
            nbajh_bytecode_compiler_fini(&compiler);
            return error;
        }
    }

    *last_token_dst = compiler.token;
    size_t loop_stack_size = compiler.loop_stack.size;
    nbajh_bytecode_compiler_fini(&compiler);

    if (loop_stack_size != 0) {
        return NBAJH_ERROR_COMPILER_UNCLOSED_LOOPS;
    }

    uint8_t ret[] = { NBAJH_BYTECODE_OP_RET };
    return NBAJH_BUFFER_WRITE(dst, ret);
}
