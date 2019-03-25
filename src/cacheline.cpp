#include <bits/cacheline.hpp>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

namespace bits {
namespace {

// MUST be a power of two to perform bitwise AND based modulo.
constexpr std::size_t kArrayLen = 1024 * 1024;
constexpr std::size_t kNumLoads = 1024 * 1024;
constexpr std::size_t kMaxCacheLine = 1024;
constexpr double kThresh = 1.25;

std::vector<std::uint8_t> createRandomVec(std::size_t step) {
  std::random_device r;
  std::default_random_engine eng{r()};
  std::uniform_int_distribution<std::uint8_t> dist{0, 255};

  // Allocate an array (1 MB) which is much larger than an L1 cache on modern
  // processors.
  std::vector<std::uint8_t> xs(kArrayLen);
  for (auto& x : xs) x = dist(eng);

  // The xs are a jump table. Mixing in random numbers with the desired step
  // size prevents some degree of hardware prefetching. This ensures loads are
  // performed sequentially. More info here:
  // https://stackoverflow.com/a/12847535
  for (std::size_t p = 0; p < xs.size(); p += step) xs[p] = step;

  return xs;
}

std::chrono::high_resolution_clock::duration benchWithStepSize(
    std::size_t step, volatile std::size_t* doNotOptimize) {
  auto xs = createRandomVec(step);
  std::size_t p = 0;

  auto begin = std::chrono::high_resolution_clock::now();
  for (std::size_t load = 0; load < kNumLoads; load++)
    p = (p + xs[p]) & (kArrayLen - 1);
  auto loopTime = std::chrono::high_resolution_clock::now() - begin;

  // This is mostly just a trick to prevent compiler optimization of the loop.
  *doNotOptimize = p;
  return loopTime;
}

}  // namespace

boost::optional<std::size_t> getCacheLineSize() {
  static auto kCacheLineSize = []() -> boost::optional<std::size_t> {
    volatile std::size_t doNotOptimize;
    auto baseline = benchWithStepSize(1, &doNotOptimize);
    for (std::size_t cacheLineSize = 2; cacheLineSize < kMaxCacheLine;
         cacheLineSize *= 2) {
      if (benchWithStepSize(cacheLineSize, &doNotOptimize) > baseline * kThresh)
        return cacheLineSize;
    }
    return boost::none;
  }();

  return kCacheLineSize;
}

}  // namespace bits
