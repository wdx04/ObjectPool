#include <memory>
#include <chrono>
#include <thread>
#include <tbb/concurrent_queue.h>
#include <tbb/tbb_thread.h>

template <class T>
class ObjectPool
{
 private:
  struct External_Deleter {
    explicit External_Deleter(std::weak_ptr<ObjectPool<T>* > pool)
        : pool_(pool) {}

    void operator()(T* ptr) {
      if (auto pool_ptr = pool_.lock()) {
        try {
          (*pool_ptr.get())->add(std::unique_ptr<T>{ptr});
          return;
        } catch(...) {}
      }
      std::default_delete<T>{}(ptr);
    }
   private:
    std::weak_ptr<ObjectPool<T>* > pool_;
  };

 public:
  using ptr_type = std::unique_ptr<T, External_Deleter >;

  ObjectPool() : this_ptr_(new ObjectPool<T>*(this)) {}
  virtual ~ObjectPool(){}

  void add(std::unique_ptr<T>&& t) {
    pool_.push(std::move(t));
  }

  ptr_type acquire() {
	std::unique_ptr<T> ptr;
	while (!pool_.try_pop(ptr)) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return ptr_type(ptr.release(), External_Deleter{ std::weak_ptr<ObjectPool<T>*>{this_ptr_} });
  }

  void clear()
  {
	  pool_.clear();
  }

  bool empty() const {
    return pool_.empty();
  }

  size_t size() const {
    return pool_.unsafe_size();
  }

 private:
  std::shared_ptr<ObjectPool<T>* > this_ptr_;
  tbb::concurrent_queue<std::unique_ptr<T> > pool_;
};
