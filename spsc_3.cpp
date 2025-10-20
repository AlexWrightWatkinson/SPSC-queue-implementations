#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include <cassert>
#include <cstdint>
#include <immintrin.h>

// next power of two helper
static inline size_t next_power_of_two(size_t v)
{
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

struct SPSCQueue
{
  const size_t capacity;
  const size_t mask;
  std::vector<int> buffer;

  alignas(64) std::atomic<size_t> head;
  char pad1[64 - sizeof(std::atomic<size_t>)];
  alignas(64) std::atomic<size_t> tail;
  char pad2[64 - sizeof(std::atomic<size_t>)];

  SPSCQueue(size_t requested_size)
    : capacity(next_power_of_two(requested_size)),
      mask(capacity - 1),
      buffer(capacity),
      head(0),
      tail(0)
  {
    assert((capacity & (capacity -1)) == 0);
  }

  bool push(int item)
  {
    const size_t t = tail.load(std::memory_order_relaxed);
    const size_t next = (t + 1) & mask;
    const size_t h = head.load(std::memory_order_acquire);
    if (next == h) return false;
    buffer[t] = item;
    tail.store(next, std::memory_order_release);
    return true;
  }

  bool pop(int &item)
  {
    const size_t h = head.load(std::memory_order_relaxed);
    const size_t t = tail.load(std::memory_order_acquire);
    if(h == t) return false;
    item = buffer[h];
    head.store((h + 1) & mask, std::memory_order_release);
    return true;
  }
};

int main()
{
  SPSCQueue q(1024);
  std::atomic<bool> done{false};

    std::thread prod([&]()
    {
      for (int i = 1; i <= 10000; ++i)
      {
        while (!q.push(i))
        {
          _mm_pause();
        }
      }
      done.store(true, std::memory_order_release);
    });

    std::thread cons([&]()
    {
      int value;
      while (true)
      {
        if (q.pop(value))
        {
        std::cout << value << "\n";
        }
        else if (done.load(std::memory_order_acquire))
        {
          break;
        }
        else
        {
          _mm_pause();
        }
      }
    });

    prod.join();
    cons.join();
    std::cout << "Completed:\n";
    return 0;
}
