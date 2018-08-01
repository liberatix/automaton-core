#ifndef AUTOMATON_CORE_STORAGE_PERSISTENT_STORAGE_H__
#define AUTOMATON_CORE_STORAGE_PERSISTENT_STORAGE_H__

#include <boost/iostreams/device/mapped_file.hpp>
#include <string>

namespace automaton {
namespace core {
namespace storage {

/**
  Storage interface for fixed size data using memory mapped file.
*/
class persistent_storage {
 public:
  explicit persistent_storage(size_t object_size);
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
    Frees allocated blob.

    @returns    bool      False if there is no allocated blob with the given id
    @param[in]  id        The ID returned by create_blob
  */
  bool free(const uint32_t id);
  /**
    TODO(Samir): document
  */
  bool map_file(std::string path);

 private:
  uint8_t* storage;
  boost::iostreams::mapped_file mmf;
  bool is_mapped = false;
  size_t object_size;

  std::string file_path;
  uint64_t next_free = 0;
  
  uint64_t capacity;
  // TODO(Samir): Handle free locations.
  // One option is to use heap with the free locations stored by size
  // Second option is to use linked list linking to the next free location
  //  if next.location.ID == this.location.ID + length:
  //    this.location.length += next.location.length
  //  while

  /**
  Creates a blob with a given size

  @returns    uint8_t*  the pointer to the created blob
  @param[in]  size      The size in bytes to be allocated
  @param[out] id        id used to get access to the blob.
  */
  uint8_t* create_blob(uint32_t const size, uint64_t* id);
};

}  // namespace storage
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_STORAGE_PERSISTENT_STORAGE_H__
