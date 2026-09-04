// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#ifndef NBAJH_CONTEXT_H
#define NBAJH_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

// The size of the working memory stored in a Nothing Beats a Jet2 Holiday program context.
#define NBAJH_CONTEXT_MEMORY_SIZE (1U << 16U)

// A Nothing Beats a Jet2 Holiday program context.
struct nbajh_context {
    const uint8_t *ip;
    uint8_t *dp;
    FILE *in;
    FILE *out;
    void (*debug_handler)(struct nbajh_context *);
    const uint8_t *program;
    uint8_t memory[NBAJH_CONTEXT_MEMORY_SIZE];
};

// Initializes the given context for execution of the code stored at `program`,
// with the files pointed to by `in` and `out` as input and output files
// respectively, and the function pointed to by `debug_handler` as debugging
// event handler.
void nbajh_context_init(
    struct nbajh_context *ctx,
    const uint8_t *program,
    FILE *in,
    FILE *out,
    void (*debug_handler)(struct nbajh_context *)
);

#ifdef __cplusplus
}
#endif

#endif // NBAJH_CONTEXT_H
