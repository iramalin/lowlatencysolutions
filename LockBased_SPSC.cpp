#include <iostream>
#include <semaphore.h>
#include <thread>
#include <chrono>

template <class T>
class CQueue
{
  private:
    static const std::size_t _capacity = 1024;
    T _queue[_capacity];
    sem_t _writeSem;
    sem_t _readSem;
    std::size_t _writeIndex;
    std::size_t _readIndex;
  public:
    CQueue()
      : _writeIndex(0), _readIndex(0)
    { 
      sem_init(&_writeSem, 0, _capacity);
      sem_init(&_readSem, 0, 0);
    }
    void push(T value)
    {
      sem_wait(&_writeSem);
      _queue[_writeIndex] = value;
      sem_post(&_readSem);
      _writeIndex = (_writeIndex + 1) % _capacity;
    }
    T pop()
    {
      sem_wait(&_readSem);
      T value = _queue[_readIndex];
      sem_post(&_writeSem);
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

/*iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ g++ LockBased_SPSC.cpp -pthread -std=c++14
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 251ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 198ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 207ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 252ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 214ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 263ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 208ms
 *iramalin@iramalin-VirtualBox:~/git/lowlatencysolutions$ ./a.out
 *Time taken = 182ms
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
