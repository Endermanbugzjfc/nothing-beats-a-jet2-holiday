# Copyright (C) 2022 OverMighty
# SPDX-License-Identifier: GPL-3.0-only

add_executable(nothing-beats-a-jet2-holiday compile_and_run.c main.c debug_handler.c)

target_compile_features(nothing-beats-a-jet2-holiday PUBLIC c_std_99)
target_compile_options(
    nothing-beats-a-jet2-holiday PRIVATE
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall -Wextra -pedantic>
)

if(NBAJH_USE_JIT)
    target_compile_definitions(nothing-beats-a-jet2-holiday PUBLIC NBAJH_USE_JIT)
endif()

configure_file(version.h.in version.h)
target_include_directories(nothing-beats-a-jet2-holiday PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(nothing-beats-a-jet2-holiday PRIVATE nbajh)

set_target_properties(
    nothing-beats-a-jet2-holiday
    PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}
)

install(
    TARGETS nothing-beats-a-jet2-holiday
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
)
