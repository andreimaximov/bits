#include <array>
#include <cstdint>
#include <cstdlib>

#include <benchmark/benchmark.h>

namespace bits {

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

/**
 * 2018-12-23 12:03:04
 * Running ./cpp-bench
 * Run on (6 X 4100 MHz CPU s)
 * CPU Caches:
 *   L1 Data 32K (x6)
 *   L1 Instruction 32K (x6)
 *   L2 Unified 256K (x6)
 *   L3 Unified 9216K (x1)
 * ***WARNING*** CPU scaling is enabled, the benchmark real time measurements
 * may be noisy and will incur extra overhead.
 * --------------------------------------------------------
 * Benchmark                 Time           CPU Iterations
 * --------------------------------------------------------
 * bench_dispatch_1    1279767 ns    1279753 ns 541
 *
 * Performance counter stats for './cpp-bench
 * --benchmark_filter=bench_dispatch_1':
 *
 *      1012.262462      task-clock (msec)         #    0.982 CPUs utilized
 *                2      context-switches          #    0.002 K/sec
 *                0      cpu-migrations            #    0.000 K/sec
 *            2,122      page-faults               #    0.002 M/sec
 *    4,080,479,093      cycles                    #    4.031 GHz
 *    7,185,732,298      instructions              #    1.76  insn per cycle
 *    1,632,414,381      branches                  # 1612.639 M/sec
 *           55,946      branch-misses             #    0.00% of all branches
 *
 *     1.030974706 seconds time elapsed
 */
static void bench_dispatch_1(benchmark::State& state) {
  for (auto _ : state) {
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

/**
 * 2018-12-23 12:00:25
 * Running ./cpp-bench
 * Run on (6 X 4100 MHz CPU s)
 * CPU Caches:
 *   L1 Data 32K (x6)
 *   L1 Instruction 32K (x6)
 *   L2 Unified 256K (x6)
 *   L3 Unified 9216K (x1)
 * ***WARNING*** CPU scaling is enabled, the benchmark real time measurements
 * may be noisy and will incur extra overhead.
 * -------------------------------------------------------
 * Benchmark                 Time           CPU Iterations
 * --------------------------------------------------------
 * bench_dispatch_2    1269583 ns    1269576 ns 550`
 *
 * Performance counter stats for './cpp-bench
 * --benchmark_filter=bench_dispatch_2':
 *
 *     1022.212799      task-clock (msec)         #    0.982 CPUs utilized
 *                2      context-switches          #    0.002 K/sec
 *                0      cpu-migrations            #    0.000 K/sec
 *            2,120      page-faults               #    0.002 M/sec
 *    4,119,111,087      cycles                    #    4.030 GHz
 *    7,284,703,462      instructions              #    1.77  insn per cycle
 *    2,315,914,268      branches                  # 2265.589 M/sec
 *           56,188      branch-misses             #    0.00% of all branches
 *
 *      1.040700105 seconds time elapsed
 */
static void bench_dispatch_2(benchmark::State& state) {
  for (auto _ : state) {
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

/**
 * 2018-12-23 11:51:00
 * Running ./cpp-bench
 * Run on (6 X 4100 MHz CPU s)
 * CPU Caches:
 *   L1 Data 32K (x6)
 *   L1 Instruction 32K (x6)
 *   L2 Unified 256K (x6)
 *   L3 Unified 9216K (x1)
 * **WARNING*** CPU scaling is enabled, the benchmark real time
 * measurements may be noisy and will incur extra overhead.
 * --------------------------------------------------------
 * Benchmark                 Time           CPU Iterations
 * --------------------------------------------------------
 * bench_dispatch_3    4931754 ns    4931710 ns 142
 *
 * Performance counter stats for './cpp-bench
 * --benchmark_filter=bench_dispatch_3':
 *
 *       3829.207819      task-clock (msec)         #    0.995 CPUs utilized
 *                 4      context-switches          #    0.001 K/sec
 *                 0      cpu-migrations            #    0.000 K/sec
 *             2,119      page-faults               #    0.553 K/sec
 *    15,414,057,471      cycles                    #    4.025 GHz
 *    17,711,496,052      instructions              #    1.15  insn per cycle
 *     5,560,921,334      branches                  # 1452.238 M/sec
 *       262,796,602      branch-misses             #    4.73% of all branches
 *
 *       3.848179749 seconds time elapsed
 */
static void bench_dispatch_3(benchmark::State& state) {
  for (auto _ : state) {
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

/**
 * We can see a 0 to 4 ns virtual dispatch overhead. Benchmarking virtual
 * dispatch is hard because there are a number of factors at play:
 *
 *  - Cache misses
 *  - Branch misses
 *  - Compiler devirtualization
 *  - ABI layout
 *
 * The following provide good reading on these topics:
 *
 * https://eli.thegreenplace.net/2013/12/05/the-cost-of-dynamic-virtual-calls-vs-static-crtp-dispatch-in-c
 * https://ww2.ii.uj.edu.pl/~kapela/pn/cpp_vtable.html
 */
BENCHMARK(bench_dispatch_1);
BENCHMARK(bench_dispatch_2);
BENCHMARK(bench_dispatch_3);

}  // namespace bits
