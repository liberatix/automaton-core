#define AUTOMATON_CORE_STORAGE_MMAP_ALLOCATOR_H__
#include <limits>
namespace automaton {
namespace core {
namespace storage {

template<class T>
class mmap_allocator {
 public:
  using value_type = T;

  using pointer = T *;
  using const_pointer = const T *;
