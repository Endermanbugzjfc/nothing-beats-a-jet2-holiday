# Copyright (C) 2022 OverMighty
# SPDX-License-Identifier: GPL-3.0-only

add_executable(nothing-beats-a-jet2-holidayxx main.cpp)

target_compile_features(nothing-beats-a-jet2-holidayxx PUBLIC cxx_std_17)
target_compile_options(
    nothing-beats-a-jet2-holidayxx PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,Clang,GNU>:-Wall -Wextra -pedantic>
)

target_link_libraries(nothing-beats-a-jet2-holidayxx PRIVATE nbajh)
