#include <cpp/hello_world.hpp>

#include <benchmark/benchmark.h>

namespace cpp {

static void bench_hello_world(benchmark::State& state) {
  for (auto _ : state) {
    hello_world();
  }
}

BENCHMARK(bench_hello_world);

}  // namespace cpp
