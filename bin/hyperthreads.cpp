#include <pthread.h>
#include <sched.h>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

#include <boost/optional.hpp>

namespace {

constexpr std::size_t kBufLen = 1'000'000;

struct Sibling {
  std::uint64_t core;
  std::uint64_t timeUs;
};

template <typename F>
std::uint64_t timeUs(F&& f) {
  auto begin = std::chrono::steady_clock::now();
  f();
  auto end = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
      .count();
}

std::vector<double> createWorkload() {
  std::vector<double> buf(kBufLen);
  double s = 1.0;
  for (double& d : buf) {
    d = s;
    s *= 2;
  }
  return buf;
}

void runWorkload(double* buf) {
  for (auto p = buf; p < buf + kBufLen; p++) *p = std::sqrt(*p);
}

std::thread runWorkloadOnCore(double* buf, std::uint64_t core) {
  std::thread thread{[buf]() { runWorkload(buf); }};

#if defined(__linux)
  ::cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core, &cpuset);

  auto code =
      ::pthread_setaffinity_np(thread.native_handle(), sizeof(cpuset), &cpuset);
  if (code != 0) throw std::runtime_error{"Error setting thread affinity."};
#else
  throw std::runtime_error{
      "Thread affinities are not supported on your platform."};
#endif

  return thread;
}

boost::optional<Sibling> getSiblingCore(std::uint64_t core) {
  auto buf = createWorkload();
  auto base =
      timeUs([&buf, core]() { runWorkloadOnCore(buf.data(), core).join(); });

  auto cores = std::thread::hardware_concurrency();
  for (std::uint64_t cand = 0; cand < cores; cand++) {
    if (cand == core) continue;
    auto bufLhs = createWorkload();
    auto bufRhs = createWorkload();
    auto t = timeUs([&bufLhs, &bufRhs, core, cand]() {
      auto lhs = runWorkloadOnCore(bufLhs.data(), core);
      auto rhs = runWorkloadOnCore(bufRhs.data(), cand);
      lhs.join();
      rhs.join();
    });
    auto r = static_cast<double>(t) / base;
    if (r > 1.25) return Sibling{cand, t};
  }
  return boost::none;
}

}  // namespace

// This program can detect virtual cores which are "siblings" aka reside on the
// same physical core and cooperate via hyperthreading. This is done by running
// a CPU heavy workload on all pairs of cores and detecting the slowest pairs.
//
// https://eli.thegreenplace.net/2016/c11-threads-affinity-and-hyperthreading/
int main(int argc, char* argv[]) {
  auto buf = createWorkload();
  auto t = timeUs([&buf]() { runWorkload(buf.data()); });
  std::cout << "Single-Threaded workload took: " << t << " us." << std::endl;

  auto cores = std::thread::hardware_concurrency();
  for (std::uint64_t core = 0; core < cores; core++) {
    auto sibling = getSiblingCore(core);
    if (sibling) {
      std::cout << core << ": " << sibling->core
                << " is a sibling. Workload took " << sibling->timeUs << " us."
                << std::endl;
    } else {
      std::cout << core << ": No sibling." << std::endl;
    }
  }

  return 0;
}
