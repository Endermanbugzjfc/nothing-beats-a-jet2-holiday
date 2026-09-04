// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#ifndef NBAJH_TARGETS_H
#define NBAJH_TARGETS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "buffer.h"
#include "context.h"
#include "errors.h"
#include "token.h"

#include <stdbool.h>
#include <stdio.h>

// A Nothing Beats a Jet2 Holiday compilation target.
enum nbajh_target {
    // Nothing Beats a Jet2 Holiday bytecode.
    NBAJH_TARGET_BYTECODE,
    // JIT-compiled x86-64 code following the System V AMD64/x86-64 ABI's
    // calling convention.
    NBAJH_TARGET_JIT_X86_64,
};

// Returns the name of the given target as a string.
const char *nbajh_target_name(enum nbajh_target target);

// Returns true if the given target is a JIT compilation target, otherwise
// false.
bool nbajh_target_is_jit(enum nbajh_target target);

// Compiles for the given target the source file pointed to by `src` into
// code to write to the buffer pointed to by `dst` and writes the last token
// processed at the location pointed to by `last_token_dst`. Returns the error
// that occurred in the process.
//
// The buffer must have been initialized with `nbajh_buffer_init_jit()` if the
// target is a JIT compilation target, otherwise with `nbajh_buffer_init()`.
enum nbajh_error nbajh_compile(
    enum nbajh_target target,
    FILE *src,
    struct nbajh_buffer *dst,
    struct nbajh_token *last_token_dst
);

// Runs the program compiled for the given target from the context pointed to
// by `ctx`.
enum nbajh_error nbajh_run(enum nbajh_target target, struct nbajh_context *ctx);

#ifdef __cplusplus
}
#endif

#endif // NBAJH_TARGETS_H
