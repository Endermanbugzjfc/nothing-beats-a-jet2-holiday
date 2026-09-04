// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#include "nbajh/lexer.h"

#include "nbajh/token.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define NBAJH_LEXER_COMMENT_CHAR ';'

static bool nbajh_is_space(int ch) {
    return ch == '\t' || ch == '\n' || ch == '\r' || ch == ' ';
}

static bool nbajh_is_token_char(int ch) {
    return !nbajh_is_space(ch) && ch != NBAJH_LEXER_COMMENT_CHAR && ch != EOF;
}

void nbajh_lexer_init(struct nbajh_lexer *lexer, FILE *src) {
    lexer->src = src;
    lexer->line = 1;
    lexer->col = 0;
}

static int nbajh_lexer_next_char(struct nbajh_lexer *lexer) {
    lexer->col++;
    // FIXME: on I/O error, propagate NBAJH_ERROR_IO to the library consumer.
    return fgetc(lexer->src);
}

static int nbajh_lexer_peek_char(struct nbajh_lexer *lexer) {
    int ch = fgetc(lexer->src);
    ungetc(ch, lexer->src);
    return ch;
}

static void nbajh_lexer_skip_line(struct nbajh_lexer *lexer) {
    int ch;

    do {
        ch = nbajh_lexer_next_char(lexer);
    } while (ch != '\n' && ch != EOF);

    ungetc('\n', lexer->src);
}

static void nbajh_lexer_consume_new_line(struct nbajh_lexer *lexer) {
    lexer->line++;
    lexer->col = 0;
}

static enum nbajh_token_type nbajh_lexer_match_token(
    struct nbajh_lexer *lexer,
    const char *end,
    size_t end_len,
    enum nbajh_token_type type
) {
    for (size_t i = 0; i < end_len; i++) {
        if (nbajh_lexer_next_char(lexer) != end[i]) {
            return NBAJH_TOKEN_INVALID;
        }
    }

    if (nbajh_is_token_char(nbajh_lexer_peek_char(lexer))) {
        return NBAJH_TOKEN_INVALID;
    }

    return type;
}

#define NBAJH_LEXER_MATCH_TOKEN(lexer, end, type) \
    nbajh_lexer_match_token(lexer, end, strlen(end), type)

static enum nbajh_token_type
nbajh_lexer_next_token_type(struct nbajh_lexer *lexer, int ch) {
    switch (ch) {
    case 'd': return NBAJH_LEXER_MATCH_TOKEN(lexer, "arling", NBAJH_TOKEN_DARLING);
    case 'm': return NBAJH_LEXER_MATCH_TOKEN(lexer, "y", NBAJH_TOKEN_MY);
    case 'n': return NBAJH_LEXER_MATCH_TOKEN(lexer, "othing", NBAJH_TOKEN_NOTHING);
    case 'b': return NBAJH_LEXER_MATCH_TOKEN(lexer, "eats", NBAJH_TOKEN_BEATS);
    case 'a': return NBAJH_LEXER_MATCH_TOKEN(lexer, "", NBAJH_TOKEN_A);
    case 'j': return NBAJH_LEXER_MATCH_TOKEN(lexer, "et2", NBAJH_TOKEN_JET2);
    case 'h':
        switch (nbajh_lexer_next_char(lexer)) {
        case 'a': return NBAJH_LEXER_MATCH_TOKEN(lexer, "nd", NBAJH_TOKEN_HAND);
        case 'o':
            if (nbajh_lexer_next_char(lexer) != 'l') {
                return NBAJH_TOKEN_INVALID;
            }

            switch (nbajh_lexer_next_char(lexer)) {
            case 'd': return NBAJH_LEXER_MATCH_TOKEN(lexer, "", NBAJH_TOKEN_HOLD);
            case 'i':
                return NBAJH_LEXER_MATCH_TOKEN(lexer, "day", NBAJH_TOKEN_HOLIDAY);
            default: return NBAJH_TOKEN_INVALID;
            }
        default: return NBAJH_TOKEN_INVALID;
        }
    default: return NBAJH_TOKEN_INVALID;
    }
}

struct nbajh_token nbajh_lexer_next_token(struct nbajh_lexer *lexer) {
    int ch;

    while ((ch = nbajh_lexer_next_char(lexer)) != EOF) {
        switch (ch) {
        case NBAJH_LEXER_COMMENT_CHAR: nbajh_lexer_skip_line(lexer); break;
        case '\n': nbajh_lexer_consume_new_line(lexer); break;
        default:
            if (nbajh_is_space(ch)) {
                continue;
            }

            size_t col = lexer->col;
            enum nbajh_token_type type = nbajh_lexer_next_token_type(lexer, ch);
            return (struct nbajh_token){ type, lexer->line, col };
        }
    }

    return (struct nbajh_token){ NBAJH_TOKEN_EOF, lexer->line, lexer->col };
}
