#include <atomic>

#include <benchmark/benchmark.h>
#include <boost/thread.hpp>

#include <bits/rcu.hpp>

namespace bits {

constexpr auto N = 100'000;

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
static void benchRcuSnapshot(benchmark::State& state) {
  T strategy;

  for (auto _ : state) {
    for (auto k = 0; k < N; k++) {
      benchmark::DoNotOptimize(strategy.get());
    }
  }
}

template <typename T>
static void benchRcuSync(benchmark::State& state) {
  T strategy;

  for (auto _ : state) {
    for (auto k = 0; k < N; k++) {
      strategy.set('x');
    }
  }
}

template <typename T>
static void benchRcuSyncAndSnapshot(benchmark::State& state) {
  T strategy;

  if (state.thread_index == 0) {
    for (auto _ : state) {
      for (auto k = 0; k < N; k++) {
        benchmark::DoNotOptimize(strategy.get());
      }
    }
  } else {
    for (auto _ : state) {
      for (auto k = 0; k < N; k++) {
        strategy.set('x');
      }
    }
  }
}

// clang-format off
/**
 * 2018-12-30 12:18:35
 * Running ./bits-bench
 * Run on (6 X 4100 MHz CPU s)
 * CPU Caches:
 *   L1 Data 32K (x6)
 *   L1 Instruction 32K (x6)
 *   L2 Unified 256K (x6)
 *   L3 Unified 9216K (x1)
 * -----------------------------------------------------------------------------------------------------
 * Benchmark                                                              Time           CPU Iterations
 * -----------------------------------------------------------------------------------------------------
 * benchRcuSnapshot<SharedMutexSync<char>>/threads:1                   6678 us       6678 us        103
 * benchRcuSnapshot<SharedMutexSync<char>>/threads:2                   3394 us       6788 us        102
 * benchRcuSnapshot<SharedMutexSync<char>>/threads:4                   1727 us       6907 us        100
 * benchRcuSnapshot<SharedMutexSync<char>>/threads:8                   1065 us       6907 us        104
 * benchRcuSnapshot<SharedMutexSync<char>>/threads:16                   916 us       6910 us         96
 * benchRcuSnapshot<RcuSnapshotSync<char>>/threads:1                   1247 us       1247 us        554
 * benchRcuSnapshot<RcuSnapshotSync<char>>/threads:2                    630 us       1261 us        558
 * benchRcuSnapshot<RcuSnapshotSync<char>>/threads:4                    321 us       1281 us        552
 * benchRcuSnapshot<RcuSnapshotSync<char>>/threads:8                    363 us       2599 us        312
 * benchRcuSnapshot<RcuSnapshotSync<char>>/threads:16                   225 us       1822 us        448
 * benchRcuSync<SharedMutexSync<char>>/threads:1                       6756 us       6756 us        105
 * benchRcuSync<SharedMutexSync<char>>/threads:2                       3441 us       6883 us        102
 * benchRcuSync<SharedMutexSync<char>>/threads:4                       1720 us       6878 us        100
 * benchRcuSync<SharedMutexSync<char>>/threads:8                       1050 us       6885 us        104
 * benchRcuSync<SharedMutexSync<char>>/threads:16                       941 us       6875 us         96
 * benchRcuSync<RcuSnapshotSync<char>>/threads:1                       6262 us       6262 us        113
 * benchRcuSync<RcuSnapshotSync<char>>/threads:2                       5199 us      10398 us         68
 * benchRcuSync<RcuSnapshotSync<char>>/threads:4                       4527 us      18103 us         40
 * benchRcuSync<RcuSnapshotSync<char>>/threads:8                       4473 us      28460 us         24
 * benchRcuSync<RcuSnapshotSync<char>>/threads:16                      4479 us      29653 us         32
 * benchRcuSyncAndSnapshot<SharedMutexSync<char>>/threads:2            3438 us       6876 us        102
 * benchRcuSyncAndSnapshot<SharedMutexSync<char>>/threads:4            1717 us       6866 us        104
 * benchRcuSyncAndSnapshot<SharedMutexSync<char>>/threads:8            1064 us       6892 us        104
 * benchRcuSyncAndSnapshot<SharedMutexSync<char>>/threads:16            929 us       6901 us         96
 * benchRcuSyncAndSnapshot<RcuSnapshotSync<char>>/threads:2            2417 us       4833 us        142
 * benchRcuSyncAndSnapshot<RcuSnapshotSync<char>>/threads:4            3236 us      12942 us         52
 * benchRcuSyncAndSnapshot<RcuSnapshotSync<char>>/threads:8            3819 us      25684 us         24
 * benchRcuSyncAndSnapshot<RcuSnapshotSync<char>>/threads:16           4681 us      33424 us         32
 */
// clang-format on
BENCHMARK_TEMPLATE(benchRcuSnapshot, SharedMutexSync<char>)
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(benchRcuSnapshot, RcuSnapshotSync<char>)
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(benchRcuSync, SharedMutexSync<char>)
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(benchRcuSync, RcuSnapshotSync<char>)
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(benchRcuSyncAndSnapshot, SharedMutexSync<char>)
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(2, 16);
BENCHMARK_TEMPLATE(benchRcuSyncAndSnapshot, RcuSnapshotSync<char>)
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(2, 16);

}  // namespace bits
