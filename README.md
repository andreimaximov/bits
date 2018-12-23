# README

This project contains a collection of standalone programming "bits". Most of these were done to learn something. [Meson](https://mesonbuild.com/) is used for C++ builds.

## Dependencies

- [benchmark](https://github.com/google/benchmark)
- [glog](https://github.com/google/glog)
- [googletest](https://github.com/google/googletest)

## Building

You will need [Meson](http://mesonbuild.com/) and [Ninja](https://ninja-build.org/) to build. The basic steps are:

```
meson build && cd build && ninja
```

Some special targets are provided:

- **install:** Install the library and headers on your system
- **test:** Runs unit tests, don't forget to `meson configure -Db_sanitize=address`
- **benchmark:** Runs benchmarks, don't forget to `meson configure -Dbuildtype=release`
- **format:** Runs `clang-format` on the source

