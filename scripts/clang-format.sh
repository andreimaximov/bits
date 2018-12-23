#!/bin/sh

find                               \
    "${MESON_SOURCE_ROOT}/bin"     \
    "${MESON_SOURCE_ROOT}/test"    \
    "${MESON_SOURCE_ROOT}/bench"   \
    -iname '*.cpp' -o -iname '*.hpp' | xargs clang-format -i -style=file

