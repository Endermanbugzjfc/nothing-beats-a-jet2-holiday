// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#include "nbajh/targets.h"

#include "nbajh/buffer.h"
#include "nbajh/context.h"
#include "nbajh/errors.h"
#include "nbajh/targets/bytecode.h"
#include "nbajh/targets/jit_x86_64.h"
#include "nbajh/token.h"

#include <stdio.h>

const char *nbajh_target_name(enum nbajh_target target) {
    switch (target) {
    case NBAJH_TARGET_BYTECODE: return "bytecode";
    case NBAJH_TARGET_JIT_X86_64: return "JIT x86-64";
    default: return "???";
    }
}

bool nbajh_target_is_jit(enum nbajh_target target) {
    return target == NBAJH_TARGET_JIT_X86_64;
}

enum nbajh_error nbajh_compile(
    enum nbajh_target target,
    FILE *src,
    struct nbajh_buffer *dst,
    struct nbajh_token *last_token_dst
) {
    switch (target) {
    case NBAJH_TARGET_BYTECODE:
        return nbajh_compile_bytecode(src, dst, last_token_dst);
    case NBAJH_TARGET_JIT_X86_64:
        return nbajh_compile_jit_x86_64(src, dst, last_token_dst);
    default: return NBAJH_ERROR_INVALID_TARGET;
    }
}

enum nbajh_error nbajh_run(enum nbajh_target target, struct nbajh_context *ctx) {
    switch (target) {
    case NBAJH_TARGET_BYTECODE: return nbajh_run_bytecode(ctx);
    case NBAJH_TARGET_JIT_X86_64: return nbajh_run_jit_x86_64(ctx);
    default: return NBAJH_ERROR_INVALID_TARGET;
    }
}
