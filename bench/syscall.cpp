#include <unistd.h>

#include <benchmark/benchmark.h>

namespace bits {
namespace {

constexpr auto N = 1'000;

// We can see syscalls being made via perf stat -e "syscalls:sys_enter_getpid"
// ...
//
// clang-format off
// 2019-06-05 23:52:05
// Running ./bits-bench
// Run on (6 X 4100 MHz CPU s)
// CPU Caches:
//   L1 Data 32K (x6)
//   L1 Instruction 32K (x6)
//   L2 Unified 256K (x6)
//   L3 Unified 9216K (x1)
// Load Average: 0.19, 0.30, 0.28
// -------------------------------------------------------
// Benchmark             Time             CPU   Iterations
// -------------------------------------------------------
// benchSysCall        224 ns          224 ns      3082000
//
//  Performance counter stats for './bits-bench --benchmark_filter=benchSysCall':
//
//          4,196,000      syscalls:sys_enter_getpid
//
//        0.963092373 seconds time elapsed
//
// clang-format on
//
// Removing the -e "syscalls:sys_enter_getpid" flag shows the true latency...
//
// clang-format off
// 2019-06-05 23:56:31
// Running ./bits-bench
// Run on (6 X 4100 MHz CPU s)
// CPU Caches:
//   L1 Data 32K (x6)
//   L1 Instruction 32K (x6)
//   L2 Unified 256K (x6)
//   L3 Unified 9216K (x1)
//   Load Average: 0.03, 0.14, 0.21
// -------------------------------------------------------
// Benchmark             Time             CPU   Iterations
// -------------------------------------------------------
// benchSysCall        179 ns          179 ns      3825000
//
//  Performance counter stats for './bits-bench --benchmark_filter=benchSysCall':
//
//         889.008791      task-clock (msec)         #    0.975 CPUs utilized
//                  4      context-switches          #    0.004 K/sec
//                  0      cpu-migrations            #    0.000 K/sec
//                156      page-faults               #    0.175 K/sec
//      3,611,240,241      cycles                    #    4.062 GHz
//      1,016,041,337      instructions              #    0.28  insn per cycle
//        184,270,433      branches                  #  207.276 M/sec
//          9,924,224      branch-misses             #    5.39% of all branches
//
//        0.911712588 seconds time elapsed
//
// clang-format on
void benchSysCall(benchmark::State& state) {
  while (state.KeepRunningBatch(N)) {
    for (auto k = 0; k < N; k++) {
      benchmark::DoNotOptimize(getpid());
    }
  }
}

BENCHMARK(benchSysCall);

}  // namespace
}  // namespace bits
