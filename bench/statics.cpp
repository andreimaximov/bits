#include <bits/statics.hpp>

#include <benchmark/benchmark.h>

namespace bits {

constexpr auto N = 1024 * 1024 * 1024;

static void bench_statics_shared(benchmark::State& state) {
  for (auto _ : state) {
    for (auto k = 0; k < N; k++) benchmark::DoNotOptimize(Statics::get());
  }
}

static void bench_statics_thread_local(benchmark::State& state) {
  for (auto _ : state) {
    for (auto k = 0; k < N; k++) benchmark::DoNotOptimize(Statics::getTL());
  }
}

/**
 * This benchmarks demonstrates the performance differences between (1) static
 * storage and (2) thread local static storage. The performance of TLS on Linux
 * is highly dependent on compiler options used, particularly if the library
 * defining the TLS objects is a shared or static library. In both cases
 * meson configure -Dbuildtype=release was used to build the benchmarks.
 *
 * 1. Static library via meson configure -Ddefault_library=static
 *
 * 2018-12-24 17:42:29
 * Running ./bits-bench
 * Run on (6 X 4100 MHz CPU s)
 * CPU Caches:
 *   L1 Data 32K (x6)
 *   L1 Instruction 32K (x6)
 *   L2 Unified 256K (x6)
 *   L3 Unified 9216K (x1)
 * ***WARNING*** CPU scaling is enabled, the benchmark real time measurements
 * may be noisy and will incur extra overhead.
 * ------------------------------------------------------------------
 * Benchmark                           Time           CPU Iterations
 * ------------------------------------------------------------------
 * bench_statics_shared             1063 ms       1063 ms          1
 * bench_statics_thread_local       1066 ms       1066 ms          1
 *
 * 2. Shared library via meson configure -Ddefault_library=shared
 *
 * 2018-12-24 17:43:17
 * Running ./bits-bench
 * Run on (6 X 4100 MHz CPU s)
 * CPU Caches:
 *   L1 Data 32K (x6)
 *   L1 Instruction 32K (x6)
 *   L2 Unified 256K (x6)
 *   L3 Unified 9216K (x1)
 * ***WARNING*** CPU scaling is enabled, the benchmark real time measurements
 * may be noisy and will incur extra overhead.
 * ------------------------------------------------------------------
 * Benchmark                           Time           CPU Iterations
 * ------------------------------------------------------------------
 * bench_statics_shared             1602 ms       1602 ms          1
 * bench_statics_thread_local       3469 ms       3469 ms          1
 *
 * As we can see shared library TLS suffers from a 2x (approx 1.5ns) performance
 * penalty. This is due to a ~3x increase instructions stemming from
 * __tls_get_addr on Linux. More info here:
 * http://david-grs.github.io/tls_performance_overhead_cost_linux/
 *
 * Surprisingly, performance of normal statics also suffers when using shared
 * libraries. This is due to an approx. 75% increase in instructions due to PLT
 * (Procedure Linkage Table) lookups. More info here:
 * https://kristiannielsen.livejournal.com/14038.html
 *
 * Also, I've ran these benchmarks on macOS and see similar results for shared
 * libraries. However, TLS with static libraries cannot seem to match non-TLS. I
 * haven't figured out why yet because macOs doesn't support perf :(
 */
BENCHMARK(bench_statics_shared)->Unit(benchmark::kMillisecond);
BENCHMARK(bench_statics_thread_local)->Unit(benchmark::kMillisecond);

}  // namespace bits