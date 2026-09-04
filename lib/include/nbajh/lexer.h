// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#ifndef NBAJH_LEXER_H
#define NBAJH_LEXER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "token.h"

#include <stdio.h>

// A Nothing Beats a Jet2 Holiday source code lexer.
struct nbajh_lexer {
    FILE *src;
    size_t line;
    size_t col;
};

// Initializes the given lexer for lexing of the source file pointed to by
// `src`.
void nbajh_lexer_init(struct nbajh_lexer *lexer, FILE *src);

// Returns the next token from the given lexer.
struct nbajh_token nbajh_lexer_next_token(struct nbajh_lexer *lexer);

#ifdef __cplusplus
}
#endif

#endif // NBAJH_LEXER_H
