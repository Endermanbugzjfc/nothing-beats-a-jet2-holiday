# Nothing Beats a Jet2 Holiday

> "Nothing Beats a Jet2 Holiday" but it's a Turing-complete programming language.

> [!NOTE]
> This repository is forked from https://github.com/overmighty/i-use-arch-btw to fulfil this idea that I found hilarious. If you wish to support projects like this, I would highly recommend starring the **upstream repository.** Thank you.

## Introduction

Nothing Beats a Jet2 Holiday is an esoteric programming language based on [Brainfuck](
https://en.wikipedia.org/wiki/Brainfuck) in which the commands are the following
keywords:

`darling`, `hold`, `my`, `hand`, `nothing`, `beats`, `a`, `jet2`, `holiday`.

See the [language specification](./docs/language_specification.md) for more
information.

This repository contains a [C/C++ library implementing Nothing Beats a Jet2 Holiday](./lib)
and a dependent [command-line interpreter](./cmd).

## Getting Started

### Prerequisites

- [CMake](https://cmake.org/) >= 3.23
- a C99 and C++17 compiler toolchain supported by CMake and providing POSIX
  [`unistd.h`](https://en.wikipedia.org/wiki/Unistd.h), `mmap()`, `MAP_ANON`,
  and defining `__x86_64__` when targeting x86-64

### Building

    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON ..
    $ cmake --build .

### Installation

    # cmake --install .

### Usage

#### Command-line interpreter

    $ nothing-beats-a-jet2-holiday <source file>

Try some of the [example Nothing Beats a Jet2 Holiday programs](./examples) as source files.

For details:

    $ nothing-beats-a-jet2-holiday -h

#### C/C++ library

For documentation of the public API, see the [public headers](
./lib/include/nbajh).

For example usage, see the [command-line interpreter](./cmd) and [example
libnbajh programs](./examples/libnbajh).

## License

This software is licensed under the [GNU General Public License, version 3](
./LICENSE.md).
