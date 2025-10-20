#include <iostream>
#include <vector>
#include <atomic>

struct CircularBuffer
{
  std::vector<int> buffer;
  const size_t capacity;
  std::atomic<size_t> head;
  std::atomic<size_t> tail;

  CircularBuffer(size_t size)
    : buffer(size), capacity(size), head(0), tail(0) {}

  bool is_empty() const
  {
    return head.load(std::memory_order_relaxed) ==
      tail.load(std::memory_order_relaxed);
  }

  bool is_full() const
  {
    size_t next_tail = (tail.load(std::memory_order_relaxed) + 1) % capacity;
    return next_tail == head.load(std::memory_order_relaxed);
  }

  bool push(int item)
  {
    size_t t = tail.load(std::memory_order_relaxed);
    size_t h = head.load(std::memory_order_relaxed);

    size_t next = (t + 1) % capacity;
    if (next == h) return false;

    buffer[t] = item;
    tail.store(next, std::memory_order_relaxed);
    return true;
  }

  bool pop(int &item)
  {
    size_t h = head.load(std::memory_order_relaxed);
    size_t t = tail.load(std::memory_order_relaxed);

    if (h == t) return false;

    item = buffer[h];
    head.store((h+ 1) % capacity, std::memory_order_relaxed);
    return true;
  }
};

int main()
{
  CircularBuffer cb(4);

  cb.push(10);
  cb.push(20);
  cb.push(30);

  int value;
  std::cout << "Reading:\n";
  while (cb.pop(value))
  {
    std::cout << value << "\n";
  }
}
