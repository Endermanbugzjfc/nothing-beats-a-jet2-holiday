// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#ifndef NBAJH_TARGETS_BYTECODE_H
#define NBAJH_TARGETS_BYTECODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../buffer.h"
#include "../context.h"
#include "../errors.h"
#include "../token.h"

#include <stdint.h>
#include <stdio.h>

// Nothing Beats a Jet2 Holiday bytecode opcodes.
enum {
    // Return from the program.
    NBAJH_BYTECODE_OP_RET,
    // Add to the data pointer the following `uint16_t` value.
    NBAJH_BYTECODE_OP_ADDP,
    // Subtract from the data pointer the following `uint16_t` value.
    NBAJH_BYTECODE_OP_SUBP,
    // Add to the value pointed to by the data pointer the following `uint8_t`
    // value.
    NBAJH_BYTECODE_OP_ADDV,
    // Subtract from the value pointed to by the data pointer the following
    // `uint8_t` value.
    NBAJH_BYTECODE_OP_SUBV,
    // Write the value pointed to by the data pointer as character to the output
    // file.
    NBAJH_BYTECODE_OP_WRITE,
    // Read a character from the input file into the value pointed to by the
    // data pointer.
    NBAJH_BYTECODE_OP_READ,
    // Jump if the value pointed to by the data pointer is zero to the following
    // `size_t` program offset.
    NBAJH_BYTECODE_OP_JMPZ,
    // Jump if the value pointed to by the data pointer is not zero to the
    // following `size_t` program offset.
    NBAJH_BYTECODE_OP_JMPNZ,
    // Call the debugging event handler.
    NBAJH_BYTECODE_OP_DEBUG,
};

// Returns the name of the given Nothing Beats a Jet2 Holiday bytecode opcode as a string.
const char *nbajh_bytecode_op_name(uint8_t op);

// Compiles the source file pointed to by `src` into Nothing Beats a Jet2 Holiday bytecode to
// write to the buffer pointed to by `dst` and writes the last token
// processed at the location pointed to by `last_token_dst`. Returns the error
// that occurred in the process.
enum nbajh_error nbajh_compile_bytecode(
    FILE *src,
    struct nbajh_buffer *dst,
    struct nbajh_token *last_token_dst
);

// Runs the Nothing Beats a Jet2 Holiday bytecode program from the context pointed to by
// `ctx`. Returns the error that occurred in the process.
enum nbajh_error nbajh_run_bytecode(struct nbajh_context *ctx);

#ifdef __cplusplus
}
#endif

#endif // NBAJH_TARGETS_BYTECODE_H
