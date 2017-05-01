#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

template <class T>
class CQueue
{
  private:
    static const std::size_t _capacity = 1024;
    T _queue[_capacity];
    std::size_t _writeIndex;
    std::size_t _readIndex;
    std::atomic<std::size_t> _size;

  public:
    CQueue()
      : _writeIndex(0), _readIndex(0), _size(0)
    { 
    }
    void push(T value)
    {
      std::size_t tmpSize = _size.load(std::memory_order_relaxed);
      while(tmpSize >= _capacity)
        tmpSize = _size.load(std::memory_order_acquire);
      _queue[_writeIndex] = value;
      while(!_size.compare_exchange_weak(tmpSize, tmpSize + 1,
        std::memory_order_acq_rel, std::memory_order_acquire));
      _writeIndex = (_writeIndex + 1) & (_capacity - 1);
    }
    T pop()
    {
      std::size_t tmpSize = _size.load(std::memory_order_relaxed);
      while(tmpSize <= 0)
        tmpSize = _size.load(std::memory_order_acquire);
      T value = _queue[_readIndex];
      while(!_size.compare_exchange_weak(tmpSize, tmpSize - 1,
        std::memory_order_acq_rel, std::memory_order_acquire));
      _readIndex = (_readIndex + 1) & (_capacity - 1);
      return value;
    }
};

int main()
{
  auto startTime = std::chrono::high_resolution_clock::now();
  const std::size_t dataSize = 1048576;
  CQueue<int> cqueue;
  std::thread consumer([&cqueue, dataSize]()
                       {
                         std::size_t index = 0;
                         while(true)
                         {
                           int value = cqueue.pop();
                           if(value == dataSize)
                             break;
                           if(value != index)
                             std::cout << "Received " << value << " instead of " << index << std::endl;
                           ++index;
                         }
                       });
  std::thread producer([&cqueue, dataSize]()
                       {
                         for(int i = 0; i <= dataSize; ++i)
                           cqueue.push(i);
                       });
  consumer.join();
  producer.join();
  auto endTime = std::chrono::high_resolution_clock::now();
  std::cout << "Time taken = " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() << "ms" << std::endl;
}

/*
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ g++ LockFree_acq_rel_SPSC.cpp -pthread -std=c++14
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 166ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 198ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 202ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 232ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 139ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 161ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 216ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 187ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 130ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 165ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 157ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 168ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 163ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 124ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 169ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ 
 */

