# Copyright (C) 2022 OverMighty
# SPDX-License-Identifier: GPL-3.0-only

add_library(
    nbajh
    src/buffer.c
    src/context.c
    src/errors.c
    src/lexer.c
    src/targets/bytecode.c
    src/targets/bytecode_compile.c
    src/targets/bytecode_run.c
    src/targets.c
    src/targets/jit_x86_64_compile.c
    src/targets/jit_x86_64_run.c
    src/token.c
)

target_compile_features(nbajh PUBLIC c_std_99)
target_compile_options(
    nbajh PRIVATE
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall -Wextra -pedantic>
)

configure_file(include/nbajh/version.h.in include/nbajh/version.h)
target_sources(
    nbajh PUBLIC
    FILE_SET HEADERS
    BASE_DIRS include ${CMAKE_CURRENT_BINARY_DIR}/include
    FILES
    include/nbajh/buffer.h
    include/nbajh/context.h
    include/nbajh/errors.h
    include/nbajh/lexer.h
    include/nbajh/targets/bytecode.h
    include/nbajh/targets.h
    include/nbajh/targets/jit_x86_64.h
    include/nbajh/token.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/nbajh/version.h
)

set_target_properties(
    nbajh
    PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}
)

install(
    TARGETS nbajh
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}
)
