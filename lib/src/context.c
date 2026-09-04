// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#include "nbajh/context.h"

#include <string.h>

void nbajh_context_init(
    struct nbajh_context *ctx,
    const uint8_t *program,
    FILE *in,
    FILE *out,
    void (*debug_handler)(struct nbajh_context *)
) {
    ctx->ip = program;
    ctx->dp = ctx->memory;
    ctx->in = in;
    ctx->out = out;
    ctx->debug_handler = debug_handler;
    ctx->program = program;
    memset(ctx->memory, 0, NBAJH_CONTEXT_MEMORY_SIZE);
}
