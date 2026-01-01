# raycast

[![Build Status](https://github.com/bmoneill/raycast/actions/workflows/cmake-single-platform.yml/badge.svg?branch=main)](https://github.com/bmoneill/raycast/actions/workflows/cmake-single-platform.yml)
[![Doxygen Status](https://github.com/bmoneill/raycast/actions/workflows/doxygen.yml/badge.svg?branch=main)](https://bmoneill.github.io/raycast)
[![Clang-format status](https://github.com/bmoneill/raycast/actions/workflows/clang-format.yml/badge.svg?branch=main)](https://github.com/bmoneill/raycast/actions/workflows/clang-format.yml)

This is a Raycasting engine using SDL3.



## Building

### Linux

```shell
# build library and demo
cmake -S . -B build -DTARGET_GROUP=all
cmake --build build

# install
cmake --install build
```

Demos will be located in the `build/demo` directory.

## Testing

```shell
git submodule update
cmake -S . -B build -DTARGET_GROUP=test
cmake --build build
cd build
ctest --verbose
```

## Showcase

![Textured 3D View](https://github.com/bmoneill/largegifs/blob/main/raycast-demo.gif?raw=true)

## Documentation

[Library documentation is available here](https://bmoneill.github.io/raycast/).

## Further Reading

* [Lode's Computer Graphics Tutorial](https://lodev.org/cgtutor/raycasting.html)

## Bugs

If you find a bug, submit an issue, PR, or email me with a description and/or patch.

## License

Copyright (c) 2025 Ben O'Neill <ben@oneill.sh>. This work is released under the
terms of the MIT License. See [LICENSE](LICENSE) for the license terms.
