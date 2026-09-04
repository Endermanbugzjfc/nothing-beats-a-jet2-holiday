// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#include "nbajh/context.h"
#include "nbajh/errors.h"
#include "nbajh/targets/bytecode.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static enum nbajh_error nbajh_bytecode_run_addp(struct nbajh_context *ctx) {
    uint16_t operand;
    memcpy(&operand, ctx->ip, sizeof(operand));

    if (ctx->dp - ctx->memory >= NBAJH_CONTEXT_MEMORY_SIZE - operand) {
        return NBAJH_ERROR_DP_OUT_OF_BOUNDS;
    }

    ctx->ip += sizeof(operand);
    ctx->dp += operand;
    return NBAJH_ERROR_SUCCESS;
}

static enum nbajh_error nbajh_bytecode_run_subp(struct nbajh_context *ctx) {
    uint16_t operand;
    memcpy(&operand, ctx->ip, sizeof(operand));

    if (ctx->dp - ctx->memory < operand) {
        return NBAJH_ERROR_DP_OUT_OF_BOUNDS;
    }

    ctx->ip += sizeof(operand);
    ctx->dp -= operand;
    return NBAJH_ERROR_SUCCESS;
}

static enum nbajh_error nbajh_bytecode_run_write(struct nbajh_context *ctx) {
    int result = fputc(*ctx->dp, ctx->out);

    if (result == EOF) {
        return NBAJH_ERROR_IO;
    }

    return NBAJH_ERROR_SUCCESS;
}

static enum nbajh_error nbajh_bytecode_run_read(struct nbajh_context *ctx) {
    int result = fgetc(ctx->in);

    if (result == EOF) {
        if (ferror(ctx->in)) {
            return NBAJH_ERROR_IO;
        }

        return NBAJH_ERROR_RUNTIME_END_OF_INPUT_FILE;
    }

    *ctx->dp = result;
    return NBAJH_ERROR_SUCCESS;
}

static void nbajh_bytecode_run_jmpz(struct nbajh_context *ctx) {
    size_t offset;

    if (*ctx->dp != 0) {
        ctx->ip += sizeof(offset);
        return;
    }

    memcpy(&offset, ctx->ip, sizeof(offset));
    ctx->ip = ctx->program + offset;
}

static void nbajh_bytecode_run_jmpnz(struct nbajh_context *ctx) {
    size_t offset;

    if (*ctx->dp == 0) {
        ctx->ip += sizeof(offset);
        return;
    }

    memcpy(&offset, ctx->ip, sizeof(offset));
    ctx->ip = ctx->program + offset;
}

enum nbajh_error nbajh_run_bytecode(struct nbajh_context *ctx) {
    uint8_t op;

    while ((op = *ctx->ip++) != NBAJH_BYTECODE_OP_RET) {
        enum nbajh_error err = NBAJH_ERROR_SUCCESS;

        switch (op) {
        case NBAJH_BYTECODE_OP_ADDP: err = nbajh_bytecode_run_addp(ctx); break;
        case NBAJH_BYTECODE_OP_SUBP: err = nbajh_bytecode_run_subp(ctx); break;
        case NBAJH_BYTECODE_OP_ADDV: *ctx->dp += *ctx->ip++; break;
        case NBAJH_BYTECODE_OP_SUBV: *ctx->dp -= *ctx->ip++; break;
        case NBAJH_BYTECODE_OP_WRITE: err = nbajh_bytecode_run_write(ctx); break;
        case NBAJH_BYTECODE_OP_READ: err = nbajh_bytecode_run_read(ctx); break;
        case NBAJH_BYTECODE_OP_JMPZ: nbajh_bytecode_run_jmpz(ctx); break;
        case NBAJH_BYTECODE_OP_JMPNZ: nbajh_bytecode_run_jmpnz(ctx); break;
        case NBAJH_BYTECODE_OP_DEBUG: ctx->debug_handler(ctx); break;
        default: return NBAJH_ERROR_BYTECODE_INVALID_OP;
        }

        if (err != NBAJH_ERROR_SUCCESS) {
            return err;
        }
    }

    return NBAJH_ERROR_SUCCESS;
}
