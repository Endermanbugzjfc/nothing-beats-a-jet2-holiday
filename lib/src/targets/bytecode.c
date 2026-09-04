// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#include "nbajh/targets/bytecode.h"

#include <stdint.h>

const char *nbajh_bytecode_op_name(uint8_t op) {
    switch (op) {
    case NBAJH_BYTECODE_OP_RET: return "ret";
    case NBAJH_BYTECODE_OP_ADDP: return "addp";
    case NBAJH_BYTECODE_OP_SUBP: return "subp";
    case NBAJH_BYTECODE_OP_ADDV: return "addv";
    case NBAJH_BYTECODE_OP_SUBV: return "subv";
    case NBAJH_BYTECODE_OP_WRITE: return "write";
    case NBAJH_BYTECODE_OP_READ: return "read";
    case NBAJH_BYTECODE_OP_JMPZ: return "jmpz";
    case NBAJH_BYTECODE_OP_JMPNZ: return "jmpnz";
    case NBAJH_BYTECODE_OP_DEBUG: return "debug";
    default: return "???";
    }
}
