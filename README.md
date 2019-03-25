# README

This project contains a collection of standalone programming "bits". Most of these were done to learn something. [Meson](https://mesonbuild.com/) is used for C++ builds.

[![Build Status](https://travis-ci.com/andreimaximov/bits.svg?branch=master)](https://travis-ci.com/andreimaximov/bits)

## Dependencies

- [boost](https://www.boost.org/)
- [benchmark](https://github.com/google/benchmark)
- [googletest](https://github.com/google/googletest)

## Building

You will need [Meson](http://mesonbuild.com/) and [Ninja](https://ninja-build.org/) to build. The basic steps are:

```
meson build && cd build && ninja
```

Some special targets are provided:

- **test:** Runs unit tests, don't forget to `meson configure -Db_sanitize=address`
- **benchmark:** Runs benchmarks, don't forget to `meson configure -Dbuildtype=release`
- **format:** Runs `clang-format` on the source

