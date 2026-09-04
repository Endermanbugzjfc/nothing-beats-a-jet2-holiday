// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#include "nbajh/token.h"

const char *nbajh_token_type_name(enum nbajh_token_type type) {
    switch (type) {
    case NBAJH_TOKEN_EOF: return "EOF";
    case NBAJH_TOKEN_DARLING: return "darling";
    case NBAJH_TOKEN_HOLD: return "hold";
    case NBAJH_TOKEN_MY: return "my";
    case NBAJH_TOKEN_HAND: return "hand";
    case NBAJH_TOKEN_NOTHING: return "nothing";
    case NBAJH_TOKEN_BEATS: return "beats";
    case NBAJH_TOKEN_A: return "a";
    case NBAJH_TOKEN_JET2: return "jet2";
    case NBAJH_TOKEN_HOLIDAY: return "holiday";
    default: return "???";
    }
}
