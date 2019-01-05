#include <atomic>
#include <cstdint>

#include <benchmark/benchmark.h>

namespace bits {

constexpr auto N = 1'000'000;

std::atomic<std::uint64_t> x;

// clang-format off
/**
 * 2019-01-05 00:23:03
 * Running ./bits-bench
 * Run on (6 X 4100 MHz CPU s)
 * CPU Caches:
 *   L1 Data 32K (x6)
 *   L1 Instruction 32K (x6)
 *   L2 Unified 256K (x6)
 *   L3 Unified 9216K (x1)
 * --------------------------------------------------------------------
 * Benchmark                             Time           CPU Iterations
 * --------------------------------------------------------------------
 * benchAtomicsLoad/threads:6            0 ms          0 ms       2724
 *
 *  Performance counter stats for './bits-bench --benchmark_filter=benchAtomicsLoad':
 *
 *     10,179,405,073      instructions
 *            683,342      cache-references
 *            161,211      L1-dcache-load-misses
 *              5,921      LLC-load-misses
 *
 *        0.166809721 seconds time elapsed
 */
// clang-format on
static void benchAtomicsLoad(benchmark::State& state) {
  for (auto _ : state) {
    for (auto k = 0; k < N; k++)
      benchmark::DoNotOptimize(x.load(std::memory_order::memory_order_relaxed));
  }
}

// clang-format off
/**
 * 2019-01-05 00:23:15
 * Running ./bits-bench
 * Run on (6 X 4100 MHz CPU s)
 * CPU Caches:
 *   L1 Data 32K (x6)
 *   L1 Instruction 32K (x6)
 *   L2 Unified 256K (x6)
 *   L3 Unified 9216K (x1)
 * ------------------------------------------------------------------------
 * Benchmark                                 Time           CPU Iterations
 * ------------------------------------------------------------------------
 * benchAtomicsFetchOr/threads:6            26 ms        156 ms          6
 *
 *  Performance counter stats for './bits-bench --benchmark_filter=benchAtomicsFetchOr':
 *
 *         44,226,107      instructions
 *         32,043,012      cache-references
 *          5,616,075      L1-dcache-load-misses
 *              6,005      LLC-load-misses
 *
 *        0.180980093 seconds time elapsed
 */
// clang-format on
static void benchAtomicsFetchOr(benchmark::State& state) {
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
BENCHMARK(benchAtomicsLoad)->ThreadPerCpu()->Unit(benchmark::kMillisecond);
BENCHMARK(benchAtomicsFetchOr)->ThreadPerCpu()->Unit(benchmark::kMillisecond);

}  // namespace bits
