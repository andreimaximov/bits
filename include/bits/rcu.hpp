#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <thread>
#include <vector>

namespace bits {

namespace detail {

// A sharded reference counter optimized for 64 byte cache lines on x86-64. This
// offers *much* better (linear!) throughput scaling for increment/decrement
// callers (RCU readers) as the number of threads increases at the cost of (1)
// much higher memory usage and (2) slower load(...) than a single
// std::atomic<...>. On a 4-core processor this will use
// 2kb of memory, much more than 8 bytes for a single std::atomic<...>. Based on
// "A Catalog of Read Indicators":
// http://concurrencyfreaks.blogspot.com/2014/11/a-catalog-of-read-indicators.html
//
// TODO(amaximov): Figure out the cache line size programatically, perhaps by
// probing for false sharing boundaries.
class RcuRefCounter {
 public:
  RcuRefCounter() : counters_(numOfShards() * kCacheLinePad) {}

  void increment() { counters_[shardOfThread()].fetch_add(1); }

  void decrement() { counters_[shardOfThread()].fetch_sub(1); }

  std::uint64_t load() const {
    std::uint64_t sum = 0;
    for (std::size_t shard = 0; shard < numOfShards(); shard++) {
      sum += counters_[shard * kCacheLinePad].load();
    }
    return sum;
  }

 private:
  static constexpr auto kCacheLinePad =
      128 / sizeof(std::atomic<std::uint64_t>);

  static std::size_t numOfShards() {
    static const auto kShards = []() {
      // Use a power-of-two number of shards so we can use bitwise arithmetic
      // instead of modulo when figuring out the thread shard.
      std::size_t shards = 1;
      while (shards < std::thread::hardware_concurrency() * 4) shards *= 2;
      return shards;
    }();

    return kShards;
  };

  static std::size_t shardOfThread() {
    static thread_local const auto kShard = []() {
      std::hash<std::thread::id> h;
      auto k = h(std::this_thread::get_id());
      return k & (numOfShards() - 1) * kCacheLinePad;
    }();
    return kShard;
  }

  std::vector<std::atomic<std::uint64_t>> counters_;
};

template <typename Tag = void>
class RcuDomain {
 public:
  std::uint64_t lock() {
    auto v = version_.load();
    readers_[v & 1].increment();
    return v;
  }

  void unlock(std::uint64_t version) { readers_[version & 1].decrement(); }

  void sync() {
    // Ok... why does this work? Let's go step-by-step. Notice that we are using
    // sequential consistency for all atomic operations! This means there is
    // some consistent global order of operations on all threads as determined
    // at runtime.
    auto currV = version_.load();
    auto nextV = currV + 1;

    // Wait for readers still on the previous domain version (nextV & 1 doubles
    // as the previous version) to finish before trying to advance the version.
    // See the analysis below, this takes care of case (2) when CAS + following
    // loop both happen between the load(...) and fetch_add(...) in lock(...).
    while (readers_[nextV & 1].load() > 0) {
      if (version_.load() > nextV) return;
      if (version_.load() == nextV) {
        // Important to not return here! Another sync(...) thread has made
        // progress with the CAS, but maybe hasn't finished the last loop which
        // we need to run ourselves.
        break;
      }
    }

    // Ok here's the interesting bit. We could have > 1 threads in sync(...)
    // with the same nextV. The CAS is idempotent and only one thread will
    // succeed so there's no danger of double incrementing the version. We
    // should also analyze the ordering of this CAS and the following loop with
    // operations in lock(...):
    //
    // 1. CAS happens-before load(...) -> The lock(...) thread will see v =
    // nextV, thus synchronizing with the CAS and see writes made before
    // sync(...). This reader will be sync'd with the nextV version of the
    // domain so we won't wait for it. Analyzing the API usage example, this
    // means the reader is guaranteed to not see the old config pointer after
    // returning from lock(...).
    //
    // 2. CAS happens-between load(...) and fetch_add(...) -> The lock(...)
    // thread will see v = currV. If fetch_add(...) happens-before the loop, the
    // loop will see the incremented reference count and wait for the reader. If
    // fetch_add(...) happens-after the loop, fetch_add(...) will synchronize
    // with the CAS and see writes made before sync(...). Even though the return
    // from lock(...) will be that of the old version, we will still be sync'd
    // properly. We just need to wait on these readers when bumping the next
    // version (hence the loop above). Just as in case (1) , this means the
    // reader is guaranteed to not see the old config pointer after returning
    // from lock(...).
    //
    // 3. CAS happens-after fetch_add(...) -> The lock(...) thread will see v =
    // currV and fetch_add(...) will ensure the following loop sees the
    // incremented reference count and wait for the reader. Just as in case (1)
    // and (2) , this means the reader is guaranteed to not see the old config
    // pointer after returning from lock(...).
    auto currVTmp = currV;
    version_.compare_exchange_strong(currVTmp, nextV);

    // See the analysis above, this takes care of cases (2) and (3).
    while (readers_[currV & 1].load() > 0) {
      if (version_.load() > nextV) return;
    }
  }

  static RcuDomain<Tag>& get() {
    static RcuDomain<Tag> d;
    return d;
  }

 private:
  std::atomic<std::uint64_t> version_{1};
  std::array<RcuRefCounter, 2> readers_{};
};

}  // namespace detail

// RCU originates from the Linux kernel and is useful for providing a lock-free,
// wait-free, memory safe, and low latency read path for objects shared across
// threads. This is a userspace implementation of an RCU API similar to one in
// the Linux kernel:
//
// https://www.kernel.org/doc/Documentation/RCU/whatisRCU.txt
//
// The implementation is based on Grace Sharing Userspace-RCU:
// https://github.com/pramalhe/ConcurrencyFreaks/blob/master/papers/gracesharingurcu-2016.pdf
//
// RcuSnapshot costs approx. 12 ns (both to create and destroy) which is approx.
// 5x faster than boost::shared_lock<boost::shared_mutex>. Throughput scales
// linearly as the number of reader threads increases. This comes at the expense
// of RcuSnapshot<>::sync() being slower than
// boost::unique_lock<boost::shared_mutex> when there is > 1 writer thread
// performing updates in busy loop. Performance of RcuSnapshot<>::sync() is
// comparable to boost::unique_lock<boost::shared_mutex> when there is just one
// writer. The optimal usecase for RCU is a read-frequent/write-infrequent
// workload.
//
// The API is similar to folly RCU:
// https://github.com/facebook/folly/blob/master/folly/synchronization/Rcu.h
//
// std::atomic<Config*> config{new Config{}};
//
// void doStuffWithConfig(const Config& config) { ... }
//
// void reader() {
//   RcuSnapshot<> snap;
//   doStuffWithConfig(*snap.get(config));
// }
//
// void writer() {
//   while (true) {
//     std::this_thread::sleep_for(std::chrono::seconds{60});
//     Config* newConfig = loadConfig();
//     Config* oldConfig = config.exchange(newConfig);
//     RcuSnapshot<>::sync();
//     delete oldConfig;
//   }
// }
//
// An optional Tag can be provided to segregate synchronization domains of
// unrelated objects.
template <typename Tag = void>
class RcuSnapshot {
 public:
  RcuSnapshot() : version_{detail::RcuDomain<Tag>::get().lock()} {}
  RcuSnapshot(const RcuSnapshot&) = delete;
  RcuSnapshot(RcuSnapshot&& other) { *this = std::move(other); }
  RcuSnapshot& operator=(const RcuSnapshot&) = delete;
  RcuSnapshot& operator=(RcuSnapshot&& other) {
    if (version_) {
      detail::RcuDomain<Tag>::get().unlock(version_);
      version_ = 0;
    }
    std::swap(version_, other.version_);
    return *this;
  }
  ~RcuSnapshot() {
    if (version_) detail::RcuDomain<Tag>::get().unlock(version_);
  }

  template <typename T>
  T* get(const std::atomic<T*>& p) const {
    return p.load();
  }

  // Synchronizes snapshots across threads by blocking until all happens-before
  // snapshots have been destroyed. All happens-after snapshots are guaranteed
  // to see writes made before sync.
  static void sync() { detail::RcuDomain<Tag>::get().sync(); }

 private:
  std::uint64_t version_ = 0;
};

}  // namespace bits
