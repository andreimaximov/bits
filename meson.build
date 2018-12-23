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

if gtest.found() and gmock.found()
    test = executable(
        'bits-test',
        ['test/main.cpp'],
        dependencies : [gtest, gmock],
    )

    test('test', test)
endif

if benchmark.found()
    bench = executable(
        'bits-bench',
        ['bench/main.cpp', 'bench/dispatch.cpp'],
        dependencies : [benchmark, threads],
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

