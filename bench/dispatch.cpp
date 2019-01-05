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

// clang-format off
/**
 * 2019-01-05 00:28:48
 * Running ./bits-bench
 * Run on (6 X 4100 MHz CPU s)
 * CPU Caches:
 *   L1 Data 32K (x6)
 *   L1 Instruction 32K (x6)
 *   L2 Unified 256K (x6)
 *   L3 Unified 9216K (x1)
 * --------------------------------------------------------
 * Benchmark                 Time           CPU Iterations
 * --------------------------------------------------------
 * benchDispatch1      1294498 ns    1294460 ns        539
 *
 *  Performance counter stats for './bits-bench --benchmark_filter=benchDispatch1':
 *
 *        1037.707062      task-clock (msec)         #    0.983 CPUs utilized
 *                  3      context-switches          #    0.003 K/sec
 *                  0      cpu-migrations            #    0.000 K/sec
 *              2,107      page-faults               #    0.002 M/sec
 *      4,154,881,545      cycles                    #    4.004 GHz
 *      7,164,946,273      instructions              #    1.72  insn per cycle
 *      1,627,626,972      branches                  # 1568.484 M/sec
 *             58,463      branch-misses             #    0.00% of all branches
 *
 *        1.055503026 seconds time elapsed
 */
// clang-format on
static void benchDispatch1(benchmark::State& state) {
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

// clang-format off
/**
 * 2019-01-05 00:28:52
 * Running ./bits-bench
 * Run on (6 X 4100 MHz CPU s)
 * CPU Caches:
 *   L1 Data 32K (x6)
 *   L1 Instruction 32K (x6)
 *   L2 Unified 256K (x6)
 *   L3 Unified 9216K (x1)
 * --------------------------------------------------------
 * Benchmark                 Time           CPU Iterations
 * --------------------------------------------------------
 * benchDispatch2      1522048 ns    1521999 ns        459
 *
 *  Performance counter stats for './bits-bench --benchmark_filter=benchDispatch2':
 *
 *        1042.577152      task-clock (msec)         #    0.979 CPUs utilized
 *                  7      context-switches          #    0.007 K/sec
 *                  0      cpu-migrations            #    0.000 K/sec
 *              2,107      page-faults               #    0.002 M/sec
 *      4,195,159,860      cycles                    #    4.024 GHz
 *      6,284,771,944      instructions              #    1.50  insn per cycle
 *      1,997,591,259      branches                  # 1916.013 M/sec
 *             57,726      branch-misses             #    0.00% of all branches
 *
 *        1.064677845 seconds time elapsed
 */
// clang-format on
static void benchDispatch2(benchmark::State& state) {
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

// clang-format off
/**
 * 2019-01-05 00:28:57
 * Running ./bits-bench
 * Run on (6 X 4100 MHz CPU s)
 * CPU Caches:
 *   L1 Data 32K (x6)
 *   L1 Instruction 32K (x6)
 *   L2 Unified 256K (x6)
 *   L3 Unified 9216K (x1)
 * --------------------------------------------------------
 * Benchmark                 Time           CPU Iterations
 * --------------------------------------------------------
 * benchDispatch3      4977155 ns    4977128 ns        140
 *
 *  Performance counter stats for './bits-bench --benchmark_filter=benchDispatch3':
 *
 *        2297.253921      task-clock (msec)         #    0.992 CPUs utilized
 *                  5      context-switches          #    0.002 K/sec
 *                  0      cpu-migrations            #    0.000 K/sec
 *              2,103      page-faults               #    0.915 K/sec
 *      9,206,766,473      cycles                    #    4.008 GHz
 *     10,576,670,807      instructions              #    1.15  insn per cycle
 *      3,319,967,111      branches                  # 1445.189 M/sec
 *        157,162,503      branch-misses             #    4.73% of all branches
 *
 *        2.315940517 seconds time elapsed
 */
// clang-format on
static void benchDispatch3(benchmark::State& state) {
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
BENCHMARK(benchDispatch1);
BENCHMARK(benchDispatch2);
BENCHMARK(benchDispatch3);

}  // namespace bits
