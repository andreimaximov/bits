#include <atomic>

#include <benchmark/benchmark.h>
#include <boost/thread.hpp>

#include <bits/rcu.hpp>

namespace bits {
namespace {

constexpr auto N = 1'000'000;

template <typename T>
class SharedMutexSync {
 public:
  T get() {
    boost::shared_lock<boost::shared_mutex> lock{mutex_};
    return x_;
  }

  void set(T x) {
    boost::unique_lock<boost::shared_mutex> lock{mutex_};
    x_ = std::move(x);
  }

 private:
  boost::shared_mutex mutex_;
  T x_;
};

template <typename T>
class RcuSnapshotSync {
 public:
  RcuSnapshotSync() { p.store(new T{}); }
  ~RcuSnapshotSync() { delete p.exchange(nullptr); }

  T get() {
    RcuSnapshot<> snap;
    return *snap.get(p);
  }

  void set(T x) {
    auto newX = new T{std::move(x)};
    auto oldX = p.exchange(newX);
    RcuSnapshot<>::sync();
    delete oldX;
  }

  std::atomic<T*> p{nullptr};
};

template <typename T>
void benchRcuSnapshot(benchmark::State& state) {
  T strategy;

  while (state.KeepRunningBatch(N)) {
    for (auto k = 0; k < N; k++) {
      benchmark::DoNotOptimize(strategy.get());
    }
  }
}

template <typename T>
void benchRcuSync(benchmark::State& state) {
  T strategy;

  while (state.KeepRunningBatch(N)) {
    for (auto k = 0; k < N; k++) {
      strategy.set('x');
    }
  }
}

template <typename T>
void benchRcuSyncAndSnapshot(benchmark::State& state) {
  T strategy;

  if (state.thread_index == 0) {
    while (state.KeepRunningBatch(N)) {
      for (auto k = 0; k < N; k++) {
        benchmark::DoNotOptimize(strategy.get());
      }
    }
  } else {
    while (state.KeepRunningBatch(N)) {
      for (auto k = 0; k < N; k++) {
        strategy.set('x');
      }
    }
  }
}

// clang-format off
// 2019-04-06 20:13:37
// Running ./bits-bench
// Run on (6 X 4100 MHz CPU s)
// CPU Caches:
//   L1 Data 32K (x6)
//   L1 Instruction 32K (x6)
//   L2 Unified 256K (x6)
//   L3 Unified 9216K (x1)
// Load Average: 0.88, 1.25, 0.74
// ----------------------------------------------------------------------------------------------------
// Benchmark                                                          Time             CPU   Iterations
// ----------------------------------------------------------------------------------------------------
// benchRcuSnapshot<SharedMutexSync<char>>/threads:1               66.9 ns         66.8 ns      8000000
// benchRcuSnapshot<SharedMutexSync<char>>/threads:2               33.6 ns         67.2 ns     10000000
// benchRcuSnapshot<SharedMutexSync<char>>/threads:4               16.9 ns         67.4 ns      8000000
// benchRcuSnapshot<SharedMutexSync<char>>/threads:8               10.5 ns         68.7 ns      8000000
// benchRcuSnapshot<SharedMutexSync<char>>/threads:16              9.16 ns         68.8 ns     16000000
// benchRcuSnapshot<RcuSnapshotSync<char>>/threads:1               12.2 ns         12.2 ns     58000000
// benchRcuSnapshot<RcuSnapshotSync<char>>/threads:2               6.27 ns         12.5 ns     56000000
// benchRcuSnapshot<RcuSnapshotSync<char>>/threads:4               3.15 ns         12.6 ns     52000000
// benchRcuSnapshot<RcuSnapshotSync<char>>/threads:8               1.94 ns         12.8 ns     40000000
// benchRcuSnapshot<RcuSnapshotSync<char>>/threads:16              2.68 ns         32.6 ns     16000000
// benchRcuSync<SharedMutexSync<char>>/threads:1                   65.6 ns         65.6 ns      9000000
// benchRcuSync<SharedMutexSync<char>>/threads:2                   33.6 ns         67.2 ns     10000000
// benchRcuSync<SharedMutexSync<char>>/threads:4                   16.9 ns         67.6 ns      8000000
// benchRcuSync<SharedMutexSync<char>>/threads:8                   10.4 ns         68.9 ns      8000000
// benchRcuSync<SharedMutexSync<char>>/threads:16                  8.96 ns         68.9 ns     16000000
// benchRcuSync<RcuSnapshotSync<char>>/threads:1                   59.9 ns         59.9 ns     11000000
// benchRcuSync<RcuSnapshotSync<char>>/threads:2                   49.6 ns         99.2 ns      6000000
// benchRcuSync<RcuSnapshotSync<char>>/threads:4                   45.5 ns          182 ns      4000000
// benchRcuSync<RcuSnapshotSync<char>>/threads:8                   42.9 ns          275 ns      8000000
// benchRcuSync<RcuSnapshotSync<char>>/threads:16                  45.8 ns          303 ns     16000000
// benchRcuSyncAndSnapshot<SharedMutexSync<char>>/threads:2        33.5 ns         67.0 ns     10000000
// benchRcuSyncAndSnapshot<SharedMutexSync<char>>/threads:4        16.9 ns         67.5 ns      8000000
// benchRcuSyncAndSnapshot<SharedMutexSync<char>>/threads:8        10.5 ns         68.7 ns      8000000
// benchRcuSyncAndSnapshot<SharedMutexSync<char>>/threads:16       9.91 ns         68.7 ns     16000000
// benchRcuSyncAndSnapshot<RcuSnapshotSync<char>>/threads:2        23.7 ns         47.4 ns     14000000
// benchRcuSyncAndSnapshot<RcuSnapshotSync<char>>/threads:4        31.5 ns          126 ns      4000000
// benchRcuSyncAndSnapshot<RcuSnapshotSync<char>>/threads:8        40.1 ns          267 ns      8000000
// benchRcuSyncAndSnapshot<RcuSnapshotSync<char>>/threads:16       47.4 ns          318 ns     16000000
// clang-format on
BENCHMARK_TEMPLATE(benchRcuSnapshot, SharedMutexSync<char>)->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(benchRcuSnapshot, RcuSnapshotSync<char>)->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(benchRcuSync, SharedMutexSync<char>)->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(benchRcuSync, RcuSnapshotSync<char>)->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(benchRcuSyncAndSnapshot, SharedMutexSync<char>)
    ->ThreadRange(2, 16);
BENCHMARK_TEMPLATE(benchRcuSyncAndSnapshot, RcuSnapshotSync<char>)
    ->ThreadRange(2, 16);

}  // namespace
}  // namespace bits
