# Raycaster

[![Build Status](https://github.com/bmoneill/raycaster/actions/workflows/cmake-single-platform.yml/badge.svg?branch=main)](https://github.com/bmoneill/raycaster/actions/workflows/cmake-single-platform.yml).

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

## Further Reading

* [Lode's Computer Graphics Tutorial](https://lodev.org/cgtutor/raycasting.html)

## Bugs

If you find a bug, submit an issue, PR, or email me with a description and/or patch.

## License

Copyright (c) 2025 Ben O'Neill <ben@oneill.sh>. This work is released under the
terms of the MIT License. See [LICENSE](LICENSE) for the license terms.
