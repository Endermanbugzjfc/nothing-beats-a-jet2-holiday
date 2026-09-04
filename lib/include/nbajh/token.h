// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#ifndef NBAJH_TOKEN_H
#define NBAJH_TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

// A type of Nothing Beats a Jet2 Holiday source code token.
enum nbajh_token_type {
    // End of file.
    NBAJH_TOKEN_EOF = -1,

    // The `darling` keyword.
    NBAJH_TOKEN_DARLING,
    // The `hold` keyword.
    NBAJH_TOKEN_HOLD,
    // The `my` keyword.
    NBAJH_TOKEN_MY,
    // The `hand` keyword.
    NBAJH_TOKEN_HAND,
    // The `nothing` keyword.
    NBAJH_TOKEN_NOTHING,
    // The `beats` keyword.
    NBAJH_TOKEN_BEATS,
    // The `a` keyword.
    NBAJH_TOKEN_A,
    // The `jet2` keyword.
    NBAJH_TOKEN_JET2,
    // The `holiday` keyword.
    NBAJH_TOKEN_HOLIDAY,

    // Invalid token type.
    NBAJH_TOKEN_INVALID,
};

// Returns the name of the given token type as a string.
const char *nbajh_token_type_name(enum nbajh_token_type type);

// A Nothing Beats a Jet2 Holiday source code token.
struct nbajh_token {
    enum nbajh_token_type type;
    size_t line;
    size_t col;
};

#ifdef __cplusplus
}
#endif

#endif
