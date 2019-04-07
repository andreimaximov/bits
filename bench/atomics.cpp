#include <atomic>
#include <cstdint>

#include <benchmark/benchmark.h>

namespace bits {
namespace {

constexpr auto N = 1'000'000;

std::atomic<std::uint64_t> x;

// clang-format off
// 2019-04-06 20:43:33
// Running ./bits-bench
// Run on (6 X 4100 MHz CPU s)
// CPU Caches:
//   L1 Data 32K (x6)
//   L1 Instruction 32K (x6)
//   L2 Unified 256K (x6)
//   L3 Unified 9216K (x1)
// Load Average: 0.44, 0.14, 0.19
// ---------------------------------------------------------------------
// Benchmark                           Time             CPU   Iterations
// ---------------------------------------------------------------------
// benchAtomicsLoad/threads:6      0.043 ns        0.257 ns   2718000000
//
//  Performance counter stats for './bits-bench --benchmark_filter=benchAtomicsLoad':
//
//     10,272,099,438      instructions
//          1,041,800      cache-references
//            242,889      L1-dcache-load-misses
//              5,822      LLC-load-misses
//
//        0.171581456 seconds time elapsed
// clang-format on
void benchAtomicsLoad(benchmark::State& state) {
  while (state.KeepRunningBatch(N)) {
    for (auto k = 0; k < N; k++)
      benchmark::DoNotOptimize(x.load(std::memory_order::memory_order_relaxed));
  }
}

// clang-format off
// 2019-04-06 20:44:05
// Running ./bits-bench
// Run on (6 X 4100 MHz CPU s)
// CPU Caches:
//   L1 Data 32K (x6)
//   L1 Instruction 32K (x6)
//   L2 Unified 256K (x6)
//   L3 Unified 9216K (x1)
// Load Average: 0.27, 0.12, 0.18
// ------------------------------------------------------------------------
// Benchmark                              Time             CPU   Iterations
// ------------------------------------------------------------------------
// benchAtomicsFetchOr/threads:6       23.8 ns          143 ns      6000000
//
//  Performance counter stats for './bits-bench --benchmark_filter=benchAtomicsFetchOr':
//
//         44,955,412      instructions
//         30,082,596      cache-references
//          5,390,323      L1-dcache-load-misses
//              4,912      LLC-load-misses
//
//        0.169033016 seconds time elapsed
// clang-format on
void benchAtomicsFetchOr(benchmark::State& state) {
  while (state.KeepRunningBatch(N)) {
    for (auto k = 0; k < N; k++)
      benchmark::DoNotOptimize(
          x.fetch_or(0, std::memory_order::memory_order_relaxed));
  }
}

// This benchmark shows the different between a simple atomic load and a
// read-modify-write operation that might need to do "more" cross core
// synchronization. We can observe a monumentally higher L1 cache miss rate when
// comparing load vs. fetch_or.
BENCHMARK(benchAtomicsLoad)->ThreadPerCpu();
BENCHMARK(benchAtomicsFetchOr)->ThreadPerCpu();

}  // namespace
}  // namespace bits
