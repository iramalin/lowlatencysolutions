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
      while(_size >= _capacity);
      _queue[_writeIndex] = value;
      ++_size;
      _writeIndex = (_writeIndex + 1) % _capacity;
    }
    T pop()
    {
      while(_size <= 0);
      T value = _queue[_readIndex];
      --_size;
      _readIndex = (_readIndex + 1) % _capacity;
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

/*iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ g++ LockFree_seq_cst_SPSC.cpp -pthread -std=c++14
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 141ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 160ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 156ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 161ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 124ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 155ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 146ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 154ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 167ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ lscpu
 *Architecture:          x86_64
 *CPU op-mode(s):        32-bit, 64-bit
 *Byte Order:            Little Endian
 *CPU(s):                4
 *On-line CPU(s) list:   0-3
 *Thread(s) per core:    1
 *Core(s) per socket:    4
 *Socket(s):             1
 *NUMA node(s):          1
 *Vendor ID:             GenuineIntel
 *CPU family:            6
 *Model:                 158
 *Model name:            Intel(R) Core(TM) i7-7700HQ CPU @ 2.80GHz
 *Stepping:              9
 *CPU MHz:               2808.004
 *BogoMIPS:              5616.00
 *Hypervisor vendor:     KVM
 *Virtualization type:   full
 *L1d cache:             32K
 *L1i cache:             32K
 *L2 cache:              256K
 *L3 cache:              6144K
 *NUMA node0 CPU(s):     0-3
 */
