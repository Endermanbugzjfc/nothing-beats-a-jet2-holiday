# Copyright (C) 2022 OverMighty
# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.23)

project(nothing-beats-a-jet2-holiday VERSION 0.1.0)

option(NBAJH_BUILD_EXAMPLES "Build the libnbajh example programs by default.")
option(
    NBAJH_USE_JIT
    "Use JIT compilation in the interpreter if available for the target system."
    ON
)

# Windows does not follow the System V ABI's calling convention, and Apple
# platforms require passing the `MAP_JIT` flag to `mmap()` and the program to
# have a relevant entitlement.
if(NBAJH_USE_JIT AND (WIN32 OR APPLE))
    set(NBAJH_USE_JIT OFF)
endif()

include(GNUInstallDirs)

add_subdirectory(lib)
add_subdirectory(cmd)

if(NBAJH_BUILD_EXAMPLES)
    add_subdirectory(examples/libnbajh)
else()
    add_subdirectory(examples/libnbajh EXCLUDE_FROM_ALL)
endif()
