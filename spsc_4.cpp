#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include <cassert>
#include <cstdint>
#include <immintrin.h>
#include <algorithm>
#include <cstring>

static inline uint64_t rdtsc_serialized()
{
  unsigned aux;
  return __rdtscp(&aux);
}

static double calibrate_tsc_frequency_seconds(double sample_seconds = 0.1)
{
  using namespace std::chrono;
  // warmup
  std::this_thread::sleep_for(milliseconds(20));
  uint64_t c0 = rdtsc_serialized();
  auto t0 =high_resolution_clock::now();
  std::this_thread::sleep_for(duration<double>(sample_seconds));
  uint64_t c1 = rdtsc_serialized();
  auto t1 = high_resolution_clock::now();
  double secs = duration_cast<duration<double>>(t1 - t0).count();
  if (secs <= 0.0) secs = sample_seconds;
  return (double)(c1 - c0) / secs;
}

static inline uint64_t percentile(const std::vector<uint64_t>& v, double p)
{
  if (v.empty()) return 0;
  double idx = (v.size() - 1) * p;
  size-t i = static_cast<size_t>(idx);
  if (i >= v.size() - 1) return v.back();
  double frac = idx - i;
  return static_cast<uint64_t>((1.0 - frac) * v[i] + frac * v[i + 1]);
}

static inline size_t next_power_of_two(size_t v) {
    if (v <= 2) return 2;
    --v;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
#if SIZE_MAX > UINT32_MAX
    v |= v >> 32;
#endif
    ++v;
    return v;
}

// message stored in ring
struct Msg
{
  uint64_t tsc;i // producer timestamp (clock cycles)
  int value;
}

struct SPSCQueue
{
  SPSCQueue(const SPSCQueue&) = delete;

}
