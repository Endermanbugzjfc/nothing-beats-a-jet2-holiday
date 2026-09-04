// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#include "nbajh/errors.h"

const char *nbajh_strerror(enum nbajh_error error) {
    switch (error) {
    case NBAJH_ERROR_SUCCESS: return "success";
    case NBAJH_ERROR_MALLOC: return "memory allocation error";
    case NBAJH_ERROR_IO: return "input/output error";
    case NBAJH_ERROR_COMPILER_INVALID_TOKEN: return "invalid token";
    case NBAJH_ERROR_COMPILER_UNEXPECTED_LOOP_END: return "unexpected loop end";
    case NBAJH_ERROR_COMPILER_UNCLOSED_LOOPS: return "unclosed loops";
    case NBAJH_ERROR_COMPILER_INTERNAL: return "internal compiler error";
    case NBAJH_ERROR_DP_OUT_OF_BOUNDS: return "data pointer out of bounds";
    case NBAJH_ERROR_RUNTIME_END_OF_INPUT_FILE: return "end of input file";
    case NBAJH_ERROR_BYTECODE_INVALID_OP: return "invalid bytecode opcode";
    case NBAJH_ERROR_JIT_JUMP_TOO_LARGE: return "jump too large";
    default: return "???";
    }
}
