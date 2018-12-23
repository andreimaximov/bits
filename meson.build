project(
    'cpp',
    'cpp',
    default_options : ['cpp_std=c++11'],
    license : 'MIT',
    version : '0.0.1',
)

glog = dependency('libglog')
gtest = dependency('gtest', required : false)
gmock = dependency('gmock', required : false)
benchmark = dependency('benchmark', required : false)
threads = dependency('threads')

incdirs = include_directories('include')

install_subdir('include/cpp', install_dir : get_option('includedir'))

lib = library(
    'cpp-lib',
    ['src/hello_world.cpp'],
    dependencies : [glog],
    include_directories : incdirs,
    install : true,
)

if gtest.found() and gmock.found()
    test = executable(
        'cpp-test',
        ['test/main.cpp', 'test/hello_world.cpp'],
        dependencies : [glog, gtest, gmock],
        include_directories : incdirs,
        link_with : lib,
    )

    test('test', test)
endif

if benchmark.found()
    bench = executable(
        'cpp-bench',
        ['bench/main.cpp', 'bench/hello_world.cpp'],
        dependencies : [benchmark, glog, threads],
        include_directories : incdirs,
        link_with : lib,
    )

    benchmark('bench', bench)
endif

executable(
    'cpp-bin',
    ['bin/hello_world.cpp'],
    dependencies : [glog],
    include_directories : incdirs,
    link_with : lib,
)

run_target('format', command : 'scripts/clang-format.sh')

