// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#ifndef NBAJH_ERRORS_H
#define NBAJH_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

// Nothing Beats a Jet2 Holiday errors.
enum nbajh_error {
    // Success.
    NBAJH_ERROR_SUCCESS,

    // Memory allocation error.
    NBAJH_ERROR_MALLOC,
    // Input/output error.
    NBAJH_ERROR_IO,

    // Invalid target.
    NBAJH_ERROR_INVALID_TARGET,

    // Invalid token.
    NBAJH_ERROR_COMPILER_INVALID_TOKEN,
    // Unexpected loop end.
    NBAJH_ERROR_COMPILER_UNEXPECTED_LOOP_END,
    // Unclosed loops.
    NBAJH_ERROR_COMPILER_UNCLOSED_LOOPS,
    // Internal compiler error.
    NBAJH_ERROR_COMPILER_INTERNAL,

    // Data pointer out of bounds.
    NBAJH_ERROR_DP_OUT_OF_BOUNDS,

    // End of input file.
    NBAJH_ERROR_RUNTIME_END_OF_INPUT_FILE,

    // Invalid bytecode opcode.
    NBAJH_ERROR_BYTECODE_INVALID_OP,

    // Jump too large.
    NBAJH_ERROR_JIT_JUMP_TOO_LARGE,
};

// Returns a description of the given error as a string.
const char *nbajh_strerror(enum nbajh_error error);

#ifdef __cplusplus
}
#endif

#endif // NBAJH_ERRORS_H
