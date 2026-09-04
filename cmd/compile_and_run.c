// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#include "debug_handler.h"
#include "log.h"

#include "nbajh/buffer.h"
#include "nbajh/context.h"
#include "nbajh/errors.h"
#include "nbajh/targets.h"
#include "nbajh/token.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__x86_64__) && defined(NBAJH_USE_JIT)
    #define COMPILE_AND_RUN_TARGET NBAJH_TARGET_JIT_X86_64
#else
    #define COMPILE_AND_RUN_TARGET NBAJH_TARGET_BYTECODE
#endif

int compile(enum nbajh_target target, FILE *src, struct nbajh_buffer *dst) {
    struct nbajh_token last_token;
    enum nbajh_error error = nbajh_compile(target, src, dst, &last_token);

    if (error != NBAJH_ERROR_SUCCESS) {
        LOG_ERROR(
            "compiler error: %s at line %zu, col %zu\n",
            nbajh_strerror(error),
            last_token.line,
            last_token.col
        );
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int run(enum nbajh_target target, const struct nbajh_buffer *program) {
    struct nbajh_context ctx;
    nbajh_context_init(&ctx, program->data, stdin, stdout, debug_handler);
    enum nbajh_error error = nbajh_run(target, &ctx);

    if (error != NBAJH_ERROR_SUCCESS) {
        LOG_ERROR(
            "run-time error: %s at %p (program + %p)\n",
            nbajh_strerror(error),
            (void *) ctx.ip,
            (void *) (ctx.ip - ctx.program)
        );
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int compile_and_run(const char *filename) {
    FILE *src = fopen(filename, "rbe");

    if (!src) {
        LOG_ERROR("failed to open source file: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    enum nbajh_target target = COMPILE_AND_RUN_TARGET;
    bool is_jit_target = nbajh_target_is_jit(target);

    struct nbajh_buffer program;
    enum nbajh_error error = nbajh_buffer_init_maybe_jit(&program, is_jit_target);

    if (error != NBAJH_ERROR_SUCCESS) {
        LOG_ERROR("failed to init program buffer: %s\n", nbajh_strerror(error));
        fclose(src);
        return EXIT_FAILURE;
    }

    int status = compile(target, src, &program);
    fclose(src);

    if (status == EXIT_SUCCESS) {
        status = run(target, &program);
    }

    nbajh_buffer_fini_maybe_jit(&program, is_jit_target);
    return status;
}
