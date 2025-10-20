#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>

struct SCPSQueue
{
  std::vector<int> buffer;
  const size_t capacity;
  std::atomic<size_t> head;
  std::atomic<size_t> tail;

  SCPSQueue(size_t size)
    : buffer(size), capacity(size), head(0), tail(0) {}

  bool push(int item)
  {
    size_t t = tail.load(std::memory_order_relaxed);
    size_t h = head.load(std::memory_order_acquire);

    size_t next = (t + 1) % capacity;
    if (next == h) return false;

    buffer[t] = item;
    tail.store(next, std::memory_order_release);
    return true;
  }

  bool pop(int &item)
  {
    size_t h = head.load(std::memory_order_relaxed);
    size_t t = tail.load(std::memory_order_acquire);

    if (h == t) return false;

    item = buffer[h];
    head.store((h + 1) % capacity, std::memory_order_release);
    return true;
  }
};

int main()
{
  SCPSQueue q(8);

  std::thread producer([&]()
    {
      for (int i = 1; i <= 10; ++i)
      {
        while (!q.push(i)) {}
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
  });

  std::thread consumer([&]()
    {
      int value;
      for (int i = 1; i <= 10; ++i)
      {
        while (!q.pop(value)) {}
        std::cout << "Received: " << value << std::endl;
      }
    });

  producer.join();
  consumer.join();

  return 0;
}

