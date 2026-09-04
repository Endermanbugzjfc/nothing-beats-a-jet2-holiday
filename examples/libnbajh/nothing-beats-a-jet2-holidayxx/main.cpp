#include "nbajh/buffer.h"
#include "nbajh/context.h"
#include "nbajh/errors.h"
#include "nbajh/targets.h"
#include "nbajh/token.h"

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

struct FileDeleter {
    void operator()(std::FILE *file) {
        std::fclose(file);
    }
};

int compile(nbajh_target target, std::FILE *src, nbajh_buffer *dst) {
    nbajh_token last_token;
    nbajh_error error = nbajh_compile(target, src, dst, &last_token);

    if (error != NBAJH_ERROR_SUCCESS) {
        std::cerr << "compile-time error: " << nbajh_strerror(error)
                  << " at line " << last_token.line << ", col "
                  << last_token.col << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void debug_handler([[maybe_unused]] nbajh_context *ctx) {
    std::cout << "debug handler called\n";
}

int run(nbajh_target target, nbajh_buffer *program) {
    nbajh_context ctx;
    nbajh_context_init(&ctx, program->data, stdin, stdout, debug_handler);
    nbajh_error error = nbajh_run(target, &ctx);

    if (error != NBAJH_ERROR_SUCCESS) {
        std::cerr << "run-time error: " << nbajh_strerror(error) << " at "
                  << std::hex << std::showbase
                  << reinterpret_cast<uintptr_t>(ctx.ip - 1) << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int compile_and_run(const char *filename) {
    std::unique_ptr<std::FILE, FileDeleter> src{std::fopen(filename, "rbe")};

    if (src == nullptr) {
        std::cerr << "failed to open source file: " << std::strerror(errno)
                  << "\n";
        return EXIT_FAILURE;
    }

    nbajh_buffer program;
    nbajh_error error = nbajh_buffer_init(&program);

    if (error != NBAJH_ERROR_SUCCESS) {
        std::cerr << "failed to init program buffer: " << nbajh_strerror(error)
                  << "\n";
        return EXIT_FAILURE;
    }

    nbajh_target target = NBAJH_TARGET_BYTECODE;

    int status = compile(target, src.get(), &program);

    if (status == EXIT_SUCCESS) {
        status = run(target, &program);
    }

    nbajh_buffer_fini(&program);
    return status;
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source file>\n";
        return EXIT_FAILURE;
    }

    return compile_and_run(argv[1]);
}
