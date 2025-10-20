#include <iostream>
#include <vector>

struct CircularBuffer
{
  std::vector<int> buffer;
  size_t capacity;
  size_t head;
  size_t tail;
  bool full;

  CircularBuffer(size_t size)
    : buffer(size), capacity(size), head(0), tail(0), full(false) {}

  bool is_empty() const
  {
    return (!full && head == tail);
  }

  bool is_full() const
  {
    return full;
  }

  bool push(int item)
  {
    if (full) return false;

    buffer[tail] = item;
    tail = (tail + 1) % capacity;

    if (tail == head) full = true;
    return true;
  }

  bool pop(int &item)
  {
    if (is_empty()) return false;

    item = buffer[head];
    head = (head + 1) % capacity;

    full = false;
    return true;
  }
};

int main()
{
  CircularBuffer cb(4);

  std::cout << "pushing 1,2,3\n";
  cb.push(1);
  cb.push(2);
  cb.push(3);

  int value;
  std::cout << "popping:\n";
  while (cb.pop(value))
  {
    std::cout << value << "\n";
  }

  return 0;
}
