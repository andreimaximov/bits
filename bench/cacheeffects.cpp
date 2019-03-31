#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

#include <benchmark/benchmark.h>

namespace bits {
namespace {

// A chunk contains a pointer to the "next" chunk and P * sizeof(Chunk<P>*)
// bytes of padding.
template <std::size_t P>
struct Chunk {
  Chunk<P>* next = nullptr;
  std::array<char, P * sizeof(Chunk<P>*)> buf;
};

static_assert(sizeof(Chunk<1>) == sizeof(void*) * 2, "Chunk has bad size.");
static_assert(sizeof(Chunk<2>) == sizeof(void*) * 3, "Chunk has bad size.");

template <std::size_t P>
std::vector<Chunk<P>> makeSeqChunks(std::size_t workingSetSize) {
  constexpr auto kChunkSize = sizeof(Chunk<P>);

  auto n = (workingSetSize + kChunkSize - 1) / kChunkSize;

  std::vector<Chunk<P>> chunks;
  while (chunks.size() < n) {
    chunks.emplace_back();
  }

  Chunk<P>* prev = &chunks.back();
  for (Chunk<P>& c : chunks) {
    prev->next = &c;
    prev = &c;
  }

  return chunks;
}

template <std::size_t P>
std::vector<Chunk<P>> makeRngChunks(std::size_t workingSetSize) {
  auto chunks = makeSeqChunks<P>(workingSetSize);
  std::random_shuffle(chunks.begin(), chunks.end());
  return chunks;
}

template <std::size_t P>
void benchWalk(const std::vector<Chunk<P>>& chunks, benchmark::State& state) {
  constexpr std::size_t kLoopBatch = 1'000'000;

  auto chunk = &chunks[0];

  while (state.KeepRunningBatch(kLoopBatch)) {
    auto hops = kLoopBatch;
    while (hops-- > 0) {
      chunk = chunk->next;
      benchmark::DoNotOptimize(chunk);
    }
  }
}

template <std::size_t P>
void benchSeqWalk(benchmark::State& state) {
  auto chunks = makeSeqChunks<P>(static_cast<std::size_t>(state.range(0)));
  benchWalk(chunks, state);
}

template <std::size_t P>
void benchRngWalk(benchmark::State& state) {
  auto chunks = makeRngChunks<P>(static_cast<std::size_t>(state.range(0)));
  benchWalk(chunks, state);
}

// Benchmarks similar to section 3.3.2  Measurements of Cache Effects from "What
// Every Programmer Should Know About Memory"
//
// https://www.akkadia.org/drepper/cpumemory.pdf
//
// These benchmarks experiments with (1) the working set size via the number of
// chunks (2) the step size via chunk padding.
//
// Increasing the working set means all of the chunks do not necessarily fit in
// L1, L2, etc. cache and keeping performance constant requires more from the
// hardware prefetcher.
//
// > The advantage of hardware prefetching is that programs do not have to be
// > adjusted. The drawbacks, as just described, are that the access patterns
// > must be trivial and that prefetching cannot happen across page boundaries.
//
// Increasing the step size means we access less cache lines in each page, thus
// limiting the hardware prefetchers ability to hide latency because it cannot
// prefetch across page boundaries.
//
// Randomizing the memory access pattern prevents the prefetcher from hiding
// memory latency once the working set exceeds the capacity of cache.
//
// clang-format off
// 2019-03-30 19:09:45
// Running ./bits-bench
// Run on (8 X 2500 MHz CPU s)
// CPU Caches:
//   L1 Data 32K (x4)
//   L1 Instruction 32K (x4)
//   L2 Unified 262K (x4)
//   L3 Unified 6291K (x1)
// ------------------------------------------------------------------
// Benchmark                           Time           CPU Iterations
// ------------------------------------------------------------------
// benchRngWalk<1>/4096                3 ns          3 ns  249000000
// benchRngWalk<1>/16384               3 ns          3 ns  254000000
// benchRngWalk<1>/65536               4 ns          4 ns  177000000
// benchRngWalk<1>/262144              5 ns          5 ns  146000000
// benchRngWalk<1>/1048576            12 ns         12 ns   61000000
// benchRngWalk<1>/4194304            14 ns         14 ns   51000000
// benchRngWalk<1>/16777216           67 ns         67 ns   13000000
// benchRngWalk<1>/67108864           83 ns         83 ns    7000000
// benchRngWalk<1>/268435456          92 ns         92 ns    7000000
// benchRngWalk<7>/4096                3 ns          3 ns  249000000
// benchRngWalk<7>/16384               3 ns          3 ns  253000000
// benchRngWalk<7>/65536               5 ns          5 ns  256000000
// benchRngWalk<7>/262144              5 ns          5 ns  135000000
// benchRngWalk<7>/1048576             9 ns          9 ns   81000000
// benchRngWalk<7>/4194304            11 ns         11 ns   53000000
// benchRngWalk<7>/16777216           61 ns         61 ns   42000000
// benchRngWalk<7>/67108864           87 ns         87 ns    6000000
// benchRngWalk<7>/268435456          93 ns         93 ns    6000000
// benchRngWalk<31>/4096               3 ns          3 ns  255000000
// benchRngWalk<31>/16384              3 ns          3 ns  255000000
// benchRngWalk<31>/65536              5 ns          5 ns  173000000
// benchRngWalk<31>/262144             5 ns          5 ns  141000000
// benchRngWalk<31>/1048576           13 ns         13 ns   99000000
// benchRngWalk<31>/4194304           13 ns         13 ns   53000000
// benchRngWalk<31>/16777216          31 ns         31 ns   23000000
// benchRngWalk<31>/67108864          31 ns         31 ns   23000000
// benchRngWalk<31>/268435456         94 ns         94 ns    7000000
// benchSeqWalk<1>/4096                3 ns          3 ns  252000000
// benchSeqWalk<1>/16384               3 ns          3 ns  247000000
// benchSeqWalk<1>/65536               3 ns          3 ns  222000000
// benchSeqWalk<1>/262144              3 ns          3 ns  225000000
// benchSeqWalk<1>/1048576             3 ns          3 ns  213000000
// benchSeqWalk<1>/4194304             3 ns          3 ns  224000000
// benchSeqWalk<1>/16777216            3 ns          3 ns  216000000
// benchSeqWalk<1>/67108864            3 ns          3 ns  199000000
// benchSeqWalk<1>/268435456           3 ns          3 ns  230000000
// benchSeqWalk<7>/4096                3 ns          3 ns  253000000
// benchSeqWalk<7>/16384               3 ns          3 ns  255000000
// benchSeqWalk<7>/65536               4 ns          4 ns  156000000
// benchSeqWalk<7>/262144              4 ns          4 ns  177000000
// benchSeqWalk<7>/1048576             4 ns          4 ns  174000000
// benchSeqWalk<7>/4194304             4 ns          4 ns  165000000
// benchSeqWalk<7>/16777216            5 ns          5 ns  100000000
// benchSeqWalk<7>/67108864            5 ns          5 ns  117000000
// benchSeqWalk<7>/268435456           6 ns          6 ns  107000000
// benchSeqWalk<31>/4096               3 ns          3 ns  254000000
// benchSeqWalk<31>/16384              3 ns          3 ns  255000000
// benchSeqWalk<31>/65536              4 ns          4 ns  155000000
// benchSeqWalk<31>/262144             4 ns          4 ns  159000000
// benchSeqWalk<31>/1048576            9 ns          9 ns   75000000
// benchSeqWalk<31>/4194304           11 ns         11 ns   56000000
// benchSeqWalk<31>/16777216          21 ns         21 ns   33000000
// benchSeqWalk<31>/67108864          22 ns         22 ns   25000000
// benchSeqWalk<31>/268435456         44 ns         44 ns   12000000
// clang-format on
BENCHMARK_TEMPLATE(benchSeqWalk, 1)
    ->RangeMultiplier(4)
    ->Range(1 << 12, 1 << 28);
BENCHMARK_TEMPLATE(benchSeqWalk, 7)
    ->RangeMultiplier(4)
    ->Range(1 << 12, 1 << 28);
BENCHMARK_TEMPLATE(benchSeqWalk, 31)
    ->RangeMultiplier(4)
    ->Range(1 << 12, 1 << 28);

BENCHMARK_TEMPLATE(benchRngWalk, 1)
    ->RangeMultiplier(4)
    ->Range(1 << 12, 1 << 28);
BENCHMARK_TEMPLATE(benchRngWalk, 7)
    ->RangeMultiplier(4)
    ->Range(1 << 12, 1 << 28);
BENCHMARK_TEMPLATE(benchRngWalk, 31)
    ->RangeMultiplier(4)
    ->Range(1 << 12, 1 << 28);

}  // namespace
}  // namespace bits
