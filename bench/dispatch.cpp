#include <array>
#include <cstdint>
#include <cstdlib>

#include <benchmark/benchmark.h>

namespace bits {
namespace {

struct Base {
  virtual ~Base() {}
  virtual void* f() = 0;
};

template <std::size_t T>
struct Derived : Base {
  void* f() override {
    static int x;
    x++;
    return &x;
  }
};

constexpr auto N = 1'000'000;

// clang-format off
// 2019-04-06 20:24:09
// Running ./bits-bench
// Run on (6 X 4100 MHz CPU s)
// CPU Caches:
//   L1 Data 32K (x6)
//   L1 Instruction 32K (x6)
//   L2 Unified 256K (x6)
//   L3 Unified 9216K (x1)
// Load Average: 0.12, 0.24, 0.42
// ---------------------------------------------------------
// Benchmark               Time             CPU   Iterations
// ---------------------------------------------------------
// benchDispatch1       1.36 ns         1.36 ns    509000000
//
//  Performance counter stats for './bits-bench --benchmark_filter=benchDispatch1':
//
//        1024.373886      task-clock (msec)         #    0.983 CPUs utilized
//                  3      context-switches          #    0.003 K/sec
//                  0      cpu-migrations            #    0.000 K/sec
//              2,113      page-faults               #    0.002 M/sec
//      4,172,897,410      cycles                    #    4.074 GHz
//      3,145,824,251      instructions              #    0.75  insn per cycle
//        941,774,012      branches                  #  919.366 M/sec
//             61,098      branch-misses             #    0.01% of all branches
//
//        1.042483640 seconds time elapsed
// clang-format on
void benchDispatch1(benchmark::State& state) {
  while (state.KeepRunningBatch(N)) {
    // Simple static dispatch...
    state.PauseTiming();
    Derived<0> d;
    std::array<Derived<0>*, N> ps;
    for (Derived<0>*& p : ps) p = &d;
    state.ResumeTiming();

    for (auto p : ps) {
      benchmark::DoNotOptimize(p->f());
    }
  }
}

// clang-format off
// 2019-04-06 20:24:14
// Running ./bits-bench
// Run on (6 X 4100 MHz CPU s)
// CPU Caches:
//   L1 Data 32K (x6)
//   L1 Instruction 32K (x6)
//   L2 Unified 256K (x6)
//   L3 Unified 9216K (x1)
// Load Average: 0.19, 0.25, 0.42
// ---------------------------------------------------------
// Benchmark               Time             CPU   Iterations
// ---------------------------------------------------------
// benchDispatch2       1.50 ns         1.50 ns    461000000
//
//  Performance counter stats for './bits-bench --benchmark_filter=benchDispatch2':
//
//        1034.923312      task-clock (msec)         #    0.983 CPUs utilized
//                  3      context-switches          #    0.003 K/sec
//                  1      cpu-migrations            #    0.001 K/sec
//              2,112      page-faults               #    0.002 M/sec
//      4,217,233,192      cycles                    #    4.075 GHz
//      6,373,851,743      instructions              #    1.51  insn per cycle
//      2,025,782,373      branches                  # 1957.423 M/sec
//             62,348      branch-misses             #    0.00% of all branches
//
//        1.052974741 seconds time elapsed
// clang-format on
void benchDispatch2(benchmark::State& state) {
  while (state.KeepRunningBatch(N)) {
    // Virtual dispatch w/just a single derived class.
    state.PauseTiming();
    Derived<0> d;
    std::array<Base*, N> ps;
    for (Base*& p : ps) p = &d;
    state.ResumeTiming();

    for (auto p : ps) {
      benchmark::DoNotOptimize(p->f());
    }
  }
}

// clang-format off
// 2019-04-06 20:24:19
// Running ./bits-bench
// Run on (6 X 4100 MHz CPU s)
// CPU Caches:
//   L1 Data 32K (x6)
//   L1 Instruction 32K (x6)
//   L2 Unified 256K (x6)
//   L3 Unified 9216K (x1)
// Load Average: 0.26, 0.26, 0.43
// ---------------------------------------------------------
// Benchmark               Time             CPU   Iterations
// ---------------------------------------------------------
// benchDispatch3       4.85 ns         4.85 ns    144000000
//
//  Performance counter stats for './bits-bench --benchmark_filter=benchDispatch3':
//
//        3893.905046      task-clock (msec)         #    0.995 CPUs utilized
//                  6      context-switches          #    0.002 K/sec
//                  0      cpu-migrations            #    0.000 K/sec
//              2,112      page-faults               #    0.542 K/sec
//     15,907,493,920      cycles                    #    4.085 GHz
//     18,273,187,408      instructions              #    1.15  insn per cycle
//      5,737,035,254      branches                  # 1473.337 M/sec
//        272,614,588      branch-misses             #    4.75% of all branches
//
//        3.912407822 seconds time elapsed
// clang-format on
void benchDispatch3(benchmark::State& state) {
  while (state.KeepRunningBatch(N)) {
    // Virtual dispatch w/two classes in a random order.
    state.PauseTiming();
    std::srand(2983498);
    Derived<0> d0;
    Derived<1> d1;
    std::array<Base*, N> ps;
    for (Base*& p : ps)
      p = (std::rand() % 2) ? static_cast<Base*>(&d0) : static_cast<Base*>(&d1);
    state.ResumeTiming();

    for (auto p : ps) {
      benchmark::DoNotOptimize(p->f());
    }
  }
}

// We can see a 0 to 4 ns virtual dispatch overhead. Benchmarking virtual
// dispatch is hard because there are a number of factors at play:
//
//  - Cache misses
//  - Branch misses
//  - Compiler devirtualization
//  - ABI layout
//
// The following provide good reading on these topics:
//
// https://eli.thegreenplace.net/2013/12/05/the-cost-of-dynamic-virtual-calls-vs-static-crtp-dispatch-in-c
// https://ww2.ii.uj.edu.pl/~kapela/pn/cpp_vtable.html
BENCHMARK(benchDispatch1);
BENCHMARK(benchDispatch2);
BENCHMARK(benchDispatch3);

}  // namespace
}  // namespace bits
