#include <atomic>
#include <cstdint>

#include <benchmark/benchmark.h>

namespace bits {

constexpr auto N = 1000000;

std::atomic<std::uint64_t> x;

/**
 * 2018-12-25 17:19:06
 * Running ./bits-bench
 * Run on (6 X 4100 MHz CPU s)
 * CPU Caches:
 *   L1 Data 32K (x6)
 *   L1 Instruction 32K (x6)
 *   L2 Unified 256K (x6)
 *   L3 Unified 9216K (x1)
 * ***WARNING*** CPU scaling is enabled, the benchmark real time measurements
 * may be noisy and will incur extra overhead.
 * --------------------------------------------------------------------
 * Benchmark                             Time           CPU Iterations
 * --------------------------------------------------------------------
 * bench_atomics_load/threads:6          0 ms          2
 *
 * Performance counter stats for './bits-bench
 * --benchmark_filter=bench_atomics_load':
 *
 * 11,324,672,900      instructions
 *        746,287      cache-references
 *        148,206      L1-dcache-load-misses
 *          5,845      LLC-load-misses
 *
 *    0.173864595 seconds time elapsed
 */
static void bench_atomics_load(benchmark::State& state) {
  for (auto _ : state) {
    for (auto k = 0; k < N; k++)
      benchmark::DoNotOptimize(x.load(std::memory_order::memory_order_relaxed));
  }
}

/**
 * 2018-12-25 17:24:14
 * Running ./bits-bench
 * Run on (6 X 4100 MHz CPU s)
 * CPU Caches:
 *   L1 Data 32K (x6)
 *   L1 Instruction 32K (x6)
 *   L2 Unified 256K (x6)
 *   L3 Unified 9216K (x1)
 * ***WARNING*** CPU scaling is enabled, the benchmark real time measurements
 * may be noisy and will incur extra overhead.
 * ------------------------------------------------------------------------
 * Benchmark                                 Time           CPU Iterations
 * ------------------------------------------------------------------------
 * bench_atomics_fetch_or/threads:6         28 ms        168 ms          6
 *
 * Performance counter stats for './bits-bench
 * --benchmark_filter=bench_atomics_fetch_or':
 *
 * 108,991,046      instructions
 *  33,424,858      cache-references
 *   6,084,362      L1-dcache-load-misses
 *       7,715      LLC-load-misses
 *
 * 0.192767959 seconds time elapsed
 */
static void bench_atomics_fetch_or(benchmark::State& state) {
  for (auto _ : state) {
    for (auto k = 0; k < N; k++)
      benchmark::DoNotOptimize(
          x.fetch_or(0, std::memory_order::memory_order_relaxed));
  }
}

/**
 * This benchmark shows the different between a simple atomic load and a
 * read-modify-write operation that might need to do "more" cross core
 * synchronization. We can observe a monumentally higher L1 cache miss rate when
 * comparing load vs. fetch_or.
 */
BENCHMARK(bench_atomics_load)->ThreadPerCpu()->Unit(benchmark::kMillisecond);
BENCHMARK(bench_atomics_fetch_or)
    ->ThreadPerCpu()
    ->Unit(benchmark::kMillisecond);

}  // namespace bits
