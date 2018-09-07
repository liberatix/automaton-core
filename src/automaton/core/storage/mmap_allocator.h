#ifndef AUTOMATON_CORE_STORAGE_MMAP_ALLOCATOR_H__
#define AUTOMATON_CORE_STORAGE_MMAP_ALLOCATOR_H__

#include <iostream>
#include <limits>
#include <memory>
#include <vector>
#include "automaton/core/storage/persistent_storage.h"

namespace automaton {
namespace core {
namespace storage {

template<class T>
class mmap_allocator {
 public:
  using value_type = T;

  using pointer = T *;
  using const_pointer = const T *;

  using size_type = size_t;

  mmap_allocator(std::string path) {
    persistent_storage bs;
    if (!bs.map_file(path, sizeof(T))) {
      throw "Could not map this file";
    }
  }

  ~mmap_allocator() = default;

  pointer allocate(size_type numObjects) {
    capacity = numObjects;
    return reinterpret_cast<pointer>(operator new(sizeof(T) * numObjects) );
  }

  void deallocate(pointer p, size_type numObjects) {
    if (p) {
      delete(p);
      std::cout << "used my_allocator to deallocate " << numObjects << " elements (-)" << std::endl;
    } 
  }

  size_type get_allocations() const {
    return capacity;
  }

 private:
  static size_type capacity;
};

template<class T>
typename mmap_allocator<T>::size_type mmap_allocator<T>::capacity = 0;


}  // namespace storage
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_STORAGE_MMAP_ALLOCATOR_H__
