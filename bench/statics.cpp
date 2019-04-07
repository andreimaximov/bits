#include <bits/statics.hpp>

#include <benchmark/benchmark.h>

namespace bits {
namespace {

constexpr auto N = 1'000'000'000;

void benchStaticsShared(benchmark::State& state) {
  while (state.KeepRunningBatch(N)) {
    for (auto k = 0; k < N; k++) benchmark::DoNotOptimize(Statics::get());
  }
}

void benchStaticsThreadLocal(benchmark::State& state) {
  while (state.KeepRunningBatch(N)) {
    for (auto k = 0; k < N; k++) benchmark::DoNotOptimize(Statics::getTL());
  }
}

// This benchmarks demonstrates the performance differences between (1) static
// storage and (2) thread local static storage. The performance of TLS on Linux
// is highly dependent on compiler options used, particularly if the library
// defining the TLS objects is a shared or static library. In both cases
// meson configure -Dbuildtype=release was used to build the benchmarks.
//
// 1. Static library via meson configure -Ddefault_library=static
//
// 2019-04-06 20:35:09
// Running ./bits-bench
// Run on (6 X 4100 MHz CPU s)
// CPU Caches:
//   L1 Data 32K (x6)
//   L1 Instruction 32K (x6)
//   L2 Unified 256K (x6)
//   L3 Unified 9216K (x1)
// Load Average: 0.12, 0.13, 0.26
// ------------------------------------------------------------------
// Benchmark                        Time             CPU   Iterations
// ------------------------------------------------------------------
// benchStaticsShared           0.984 ns        0.984 ns   1000000000
// benchStaticsThreadLocal       1.22 ns         1.22 ns   1000000000
//
// 2. Shared library via meson configure -Ddefault_library=shared
//
//
// 2019-04-06 20:35:38
// Running ./bits-bench
// Run on (6 X 4100 MHz CPU s)
// CPU Caches:
//   L1 Data 32K (x6)
//   L1 Instruction 32K (x6)
//   L2 Unified 256K (x6)
//   L3 Unified 9216K (x1)
// Load Average: 0.24, 0.16, 0.27
// ------------------------------------------------------------------
// Benchmark                        Time             CPU   Iterations
// ------------------------------------------------------------------
// benchStaticsShared            1.48 ns         1.48 ns   1000000000
// benchStaticsThreadLocal       2.94 ns         2.94 ns   1000000000
//
// As we can see shared library TLS suffers from a > 2x (approx 1.7ns)
// performance penalty. This is due to an increase instructions stemming from
// __tls_get_addr on Linux. More info here:
// http://david-grs.github.io/tls_performance_overhead_cost_linux/
//
// Surprisingly, performance of normal statics also suffers when using shared
// libraries. This is due to an increase in instructions due to PLT
// (Procedure Linkage Table) lookups. More info here:
// https://kristiannielsen.livejournal.com/14038.html
//
// Also, I've ran these benchmarks on macOS and see similar results for shared
// libraries. However, TLS with static libraries cannot seem to match non-TLS. I
// haven't figured out why yet because macOS doesn't support perf :(
BENCHMARK(benchStaticsShared);
BENCHMARK(benchStaticsThreadLocal);

}  // namespace
}  // namespace bits
