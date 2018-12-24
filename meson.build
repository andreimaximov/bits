project(
    'cpp',
    'cpp',
    default_options : ['cpp_std=c++11'],
    license : 'MIT',
    version : '0.0.1',
)

boost = dependency('boost', required : true)
gtest = dependency('gtest', required : false)
gmock = dependency('gmock', required : false)
benchmark = dependency('benchmark', required : false)
threads = dependency('threads')

incdirs = include_directories('include')

lib = library(
     'bits',
     ['src/statics.cpp'],
     dependencies : [boost, threads],
     include_directories : incdirs,
)

if gtest.found() and gmock.found()
    test = executable(
        'bits-test',
        ['test/main.cpp'],
        dependencies : [gtest, gmock],
        include_directories : incdirs,
        link_with : lib,
    )

    test('test', test)
endif

if benchmark.found()
    bench = executable(
        'bits-bench',
        ['bench/main.cpp', 'bench/dispatch.cpp', 'bench/statics.cpp'],
        dependencies : [benchmark, threads],
        include_directories : incdirs,
        link_with : lib,
    )

    benchmark('bench', bench)
endif

bin_defs = {
    'hyperthreads' : 'bin/hyperthreads.cpp',
}

bin_exes = []

foreach name, cpp : bin_defs
    bin_exes += executable(
        name,
        [cpp],
        dependencies : [boost, threads],
    )
endforeach

run_target('format', command : 'scripts/clang-format.sh')

