# Raycaster

[![Build Status](https://github.com/bmoneill/raycaster/actions/workflows/cmake-single-platform.yml/badge.svg?branch=main)](https://github.com/bmoneill/raycaster/actions/workflows/cmake-single-platform.yml)
[![Doxygen Status](https://github.com/bmoneill/raycaster/actions/workflows/doxygen.yml/badge.svg?branch=main)](https://bmoneill.github.io/raycaster)
[![Clang-format status](https://github.com/bmoneill/raycaster/actions/workflows/clang-format.yml/badge.svg?branch=main)](https://github.com/bmoneill/raycaster/actions/workflows/clang-format.yml)

This is a Raycasting engine using SDL3.



## Building

### Linux

```shell
cmake --build .
```

## Testing

```shell
cmake -DTARGET_GROUP='test' .
cmake --build .
ctest .
```

## Documentation

[Library documentation is available here](https://bmoneill.github.io/raycaster/).

## Showcase

![Textured 3D View](https://oneill.sh/img/raycast-textured.gif)

![2D View](https://oneill.sh/img/raycast-2d.gif)

## Further Reading

* [Lode's Computer Graphics Tutorial](https://lodev.org/cgtutor/raycasting.html)

## Bugs

If you find a bug, submit an issue, PR, or email me with a description and/or patch.

## License

Copyright (c) 2025 Ben O'Neill <ben@oneill.sh>. This work is released under the
terms of the MIT License. See [LICENSE](LICENSE) for the license terms.
