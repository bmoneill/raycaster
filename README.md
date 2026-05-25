<div align="center">
    <h1><b>raycast</b>
    <h4>A Raycasting engine using SDL3.</h4>
    <a href="https://github.com/bmoneill/raycast/actions/workflows/ci-linux.yml"><img src="https://github.com/bmoneill/raycast/actions/workflows/ci-linux.yml/badge.svg?branch=main" alt="Linux CI status" /></a>
    <a href="https://github.com/bmoneill/raycast/actions/workflows/ci-macos.yml"><img src="https://github.com/bmoneill/raycast/actions/workflows/ci-macos.yml/badge.svg?branch=main" alt="macOS CI status" /></a>
    <a href="https://github.com/bmoneill/raycast/actions/workflows/ci-windows.yml"><img src="https://github.com/bmoneill/raycast/actions/workflows/ci-windows.yml/badge.svg?branch=main" alt="Windows CI status" /></a>
    <a href="https://bmoneill.github.io/raycast"><img src="https://github.com/bmoneill/raycast/actions/workflows/doxygen.yml/badge.svg?branch=main" alt="Doxygen Status" /></a>
    <a href="https://github.com/bmoneill/raycast/actions/workflows/clang-format.yml"><img src="https://github.com/bmoneill/raycast/actions/workflows/clang-format.yml/badge.svg?branch=main" alt="Clang-format status" /></a>
</div>

## Table of Contents

- [Overview](#overview)
- [Building](#building)
- [Requirements](#requirements)
- [Testing](#testing)
- [Showcase](#showcase)
- [Library Documentation](#documentation)
- [Further Reading](#further-reading)
- [Bugs](#bugs)
- [License](#license)

## Overview

This is a Raycasting engine using SDL3, with support for 3-D textured and untextured
rendering, as well as 2-D top-down "minimap" rendering.

## Building

### Requirements

- [CMake](https://cmake.org/) 3.31.6 or higher
- A C99-compatible C compiler (e.g. GCC, Clang)
- [SDL3](https://wiki.libsdl.org/SDL3/FrontPage)

### Linux / macOS / Windows (WSL or MinGW)

```shell
# build library and demo
cmake -S . -B build
cmake --build build

# install
cmake --install build
```

Demos will be located in the `build/demo` directory.

## Testing

### Linux / macOS

```shell
git submodule update
cmake -S . -B build -DTEST=ON
cmake --build build
cd build
ctest --verbose
```

## Showcase

![Textured 3D View](https://github.com/bmoneill/largegifs/blob/main/raycast-demo.gif?raw=true)

## Documentation

[Library documentation is available here](https://bmoneill.github.io/raycast/).

## Further Reading

- [Lode's Computer Graphics Tutorial](https://lodev.org/cgtutor/raycasting.html)

## Bugs

If you find a bug, submit an issue, PR, or email me with a description and/or patch.

## License

Copyright (c) 2025-2026 Ben O'Neill <ben@oneill.sh>. This work is released under
the terms of the MIT License. See [LICENSE](LICENSE) for the license terms.
