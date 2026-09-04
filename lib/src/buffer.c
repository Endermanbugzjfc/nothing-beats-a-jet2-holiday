// Copyright (C) 2022 OverMighty
// SPDX-License-Identifier: GPL-3.0-only

#include "nbajh/buffer.h"

#include "nbajh/errors.h"

#include <sys/mman.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NBAJH_BUFFER_DEFAULT_CAP 64
#define NBAJH_BUFFER_GROWTH_FACTOR 2

enum nbajh_error nbajh_buffer_init(struct nbajh_buffer *buffer) {
    buffer->size = 0;
    buffer->cap = NBAJH_BUFFER_DEFAULT_CAP;
    buffer->data = malloc(buffer->cap);

    if (!buffer->data) {
        return NBAJH_ERROR_MALLOC;
    }

    return NBAJH_ERROR_SUCCESS;
}

enum nbajh_error nbajh_buffer_init_jit(struct nbajh_buffer *buffer) {
    buffer->size = 0;
    buffer->cap = sysconf(_SC_PAGESIZE);
    buffer->data = mmap(
        NULL,
        buffer->cap,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_PRIVATE | MAP_ANON,
        -1,
        0
    );

    if (buffer->data == MAP_FAILED) {
        return NBAJH_ERROR_MALLOC;
    }

    return NBAJH_ERROR_SUCCESS;
}

enum nbajh_error
nbajh_buffer_write(struct nbajh_buffer *buffer, const void *data, size_t n) {
    if (buffer->size + n > buffer->cap) {
        buffer->cap *=
            (buffer->cap + n - 1) / buffer->cap * NBAJH_BUFFER_GROWTH_FACTOR;
        uint8_t *new_data = realloc(buffer->data, buffer->cap);

        if (!new_data) {
            return NBAJH_ERROR_MALLOC;
        }

        buffer->data = new_data;
    }

    memcpy(&buffer->data[buffer->size], data, n);
    buffer->size += n;
    return NBAJH_ERROR_SUCCESS;
}

enum nbajh_error
nbajh_buffer_write_jit(struct nbajh_buffer *buffer, const void *data, size_t n) {
    if (buffer->size + n > buffer->cap) {
        size_t prev_cap = buffer->cap;
        buffer->cap *=
            (buffer->cap + n - 1) / buffer->cap * NBAJH_BUFFER_GROWTH_FACTOR;
        uint8_t *new_data = mmap(
            NULL,
            buffer->cap,
            PROT_READ | PROT_WRITE | PROT_EXEC,
            MAP_PRIVATE | MAP_ANON,
            -1,
            0
        );

        if (new_data == MAP_FAILED) {
            return NBAJH_ERROR_MALLOC;
        }

        memcpy(new_data, buffer->data, buffer->size);
        munmap(buffer->data, prev_cap);
        buffer->data = new_data;
    }

    memcpy(&buffer->data[buffer->size], data, n);
    buffer->size += n;
    return NBAJH_ERROR_SUCCESS;
}

size_t nbajh_buffer_pop_size(struct nbajh_buffer *buffer) {
    size_t top = ((size_t *) &buffer->data[buffer->size])[-1];
    buffer->size -= sizeof(top);
    return top;
}

void nbajh_buffer_fini(struct nbajh_buffer *buffer) {
    free(buffer->data);
}

void nbajh_buffer_fini_jit(struct nbajh_buffer *buffer) {
    munmap(buffer->data, buffer->cap);
}
