# ObjectPool
C++ object pool for accessing shared heavy objects in multi-threaded environment.
Based on https://stackoverflow.com/questions/27827923/c-object-pool-that-provides-items-as-smart-pointers-that-are-returned-to-pool#
External dependency: Intel TBB for concurrent_queue template class.

Example:

```C++
#include "ObjectPool.h"

// create an object pool
ObjectPool<caffe::Net<float>> my_net_pool;
std::unique_ptr<caffe::Net<float>> my_net_ptr1, my_net_ptr2;
// ...instantiate net pointers here...
// add them to the pool(all instances are supposed to have identical functionality)
my_net_pool.add(std::move(my_net_ptr1));
my_net_pool.add(std::move(my_net_ptr2));
{
    // possibly from another thread accessing my_net_pool by reference
    // select any available instance in the pool
    // will block if none available
    auto p_my_net = my_net_pool.acquire();
    // ... use p_my_net here...
    p_my_net->Forward();
    // p_my_net's destructor will return the net instance to the pool for next acquisition
}
```
