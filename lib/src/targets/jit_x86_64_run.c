// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#include "nbajh/context.h"
#include "nbajh/errors.h"

enum nbajh_error nbajh_run_jit_x86_64(struct nbajh_context *ctx) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    return ((enum nbajh_error(*)(struct nbajh_context *)) ctx->ip)(ctx);
#pragma GCC diagnostic pop
}
