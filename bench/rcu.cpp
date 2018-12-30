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
static void bench_rcu_snapshot(benchmark::State& state) {
  T strategy;

  for (auto _ : state) {
    for (auto k = 0; k < N; k++) {
      benchmark::DoNotOptimize(strategy.get());
    }
  }
}

template <typename T>
static void bench_rcu_sync(benchmark::State& state) {
  T strategy;

  for (auto _ : state) {
    for (auto k = 0; k < N; k++) {
      strategy.set('x');
    }
  }
}

template <typename T>
static void bench_rcu_sync_and_snapshot(benchmark::State& state) {
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
 * ***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
 * -----------------------------------------------------------------------------------------------------
 * Benchmark                                                              Time           CPU Iterations
 * -----------------------------------------------------------------------------------------------------
 * bench_rcu_snapshot<SharedMutexSync<char>>/threads:1                 6619 us       6619 us        103
 * bench_rcu_snapshot<SharedMutexSync<char>>/threads:2                 3367 us       6733 us        102
 * bench_rcu_snapshot<SharedMutexSync<char>>/threads:4                 1695 us       6780 us        100
 * bench_rcu_snapshot<SharedMutexSync<char>>/threads:8                 1093 us       6904 us        104
 * bench_rcu_snapshot<SharedMutexSync<char>>/threads:16                1012 us       6911 us         96
 * bench_rcu_snapshot<RcuSnapshotSync<char>>/threads:1                 1226 us       1226 us        565
 * bench_rcu_snapshot<RcuSnapshotSync<char>>/threads:2                  628 us       1256 us        558
 * bench_rcu_snapshot<RcuSnapshotSync<char>>/threads:4                  317 us       1269 us        548
 * bench_rcu_snapshot<RcuSnapshotSync<char>>/threads:8                  200 us       1281 us        544
 * bench_rcu_snapshot<RcuSnapshotSync<char>>/threads:16                 215 us       1689 us        528
 * bench_rcu_sync<SharedMutexSync<char>>/threads:1                     6620 us       6620 us        105
 * bench_rcu_sync<SharedMutexSync<char>>/threads:2                     3358 us       6716 us        104
 * bench_rcu_sync<SharedMutexSync<char>>/threads:4                     1688 us       6754 us        104
 * bench_rcu_sync<SharedMutexSync<char>>/threads:8                     1072 us       6858 us        104
 * bench_rcu_sync<SharedMutexSync<char>>/threads:16                    1007 us       6872 us         96
 * bench_rcu_sync<RcuSnapshotSync<char>>/threads:1                     6058 us       6058 us        114
 * bench_rcu_sync<RcuSnapshotSync<char>>/threads:2                     4775 us       9549 us         70
 * bench_rcu_sync<RcuSnapshotSync<char>>/threads:4                     4606 us      18419 us         40
 * bench_rcu_sync<RcuSnapshotSync<char>>/threads:8                     4508 us      28939 us         24
 * bench_rcu_sync<RcuSnapshotSync<char>>/threads:16                    4200 us      29869 us         32
 * bench_rcu_sync_and_snapshot<SharedMutexSync<char>>/threads:2        3363 us       6726 us        104
 * bench_rcu_sync_and_snapshot<SharedMutexSync<char>>/threads:4        1695 us       6781 us        100
 * bench_rcu_sync_and_snapshot<SharedMutexSync<char>>/threads:8        1070 us       6885 us        104
 * bench_rcu_sync_and_snapshot<SharedMutexSync<char>>/threads:16        977 us       6889 us         96
 * bench_rcu_sync_and_snapshot<RcuSnapshotSync<char>>/threads:2        2420 us       4840 us        146
 * bench_rcu_sync_and_snapshot<RcuSnapshotSync<char>>/threads:4        3151 us      12601 us         40
 * bench_rcu_sync_and_snapshot<RcuSnapshotSync<char>>/threads:8        4424 us      28993 us         32
 * bench_rcu_sync_and_snapshot<RcuSnapshotSync<char>>/threads:16       4453 us      31293 us         32
 */
// clang-format on
BENCHMARK_TEMPLATE(bench_rcu_snapshot, SharedMutexSync<char>)
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(bench_rcu_snapshot, RcuSnapshotSync<char>)
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(bench_rcu_sync, SharedMutexSync<char>)
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(bench_rcu_sync, RcuSnapshotSync<char>)
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(bench_rcu_sync_and_snapshot, SharedMutexSync<char>)
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(2, 16);
BENCHMARK_TEMPLATE(bench_rcu_sync_and_snapshot, RcuSnapshotSync<char>)
    ->Unit(benchmark::kMicrosecond)
    ->ThreadRange(2, 16);

}  // namespace bits
