#ifndef AUTOMATON_CORE_STORAGE_PERSISTENT_STORAGE_H__
#define AUTOMATON_CORE_STORAGE_PERSISTENT_STORAGE_H__

#include <string>
#include <boost/iostreams/device/mapped_file.hpp>

namespace automaton {
namespace core {
namespace storage {

/**
  Storage interface for fixed size data using memory mapped file.
*/
class persistent_storage {
 public:
  persistent_storage();
  ~persistent_storage();

  /**
    Stores the object pointed by data
    
    @returns   bool      false if the operation was not successful 
    @param[in] at        The location at which to store the object, should be less than capacity
    @param[in] data      Pointer to the data
  */
  bool store(const uint64_t at, const uint8_t* data);

  /**
    Used to get access to previously allocated object.

    @returns    uint8_t*  pointer to the object or nullptr if id>=capacity
    @param[in]  at        The ID returned by store
  */
  uint8_t* get(const uint64_t at);

  /**
    TODO(Samir): document
  */
  bool map_file(std::string path, size_t object_sz);

 private:
  uint8_t* storage;
  boost::iostreams::mapped_file mmf;
  bool is_mapped = false;
  size_t object_size;
  size_t header_size;
  uint64_t cur_version;
  uint64_t header_version;

  std::string file_path;
  uint64_t next_free = 0;

  uint64_t capacity;
  // TODO(Samir): Handle free locations.
  // One option is to use heap with the free locations stored by size
  // Second option is to use linked list linking to the next free location
  //  if next.location.ID == this.location.ID + length:
  //    this.location.length += next.location.length
  //  while

  void close_mapped_file();
  void open_mapped_file();
};

}  // namespace storage
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_STORAGE_PERSISTENT_STORAGE_H__
