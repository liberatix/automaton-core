#ifndef AUTOMATON_CORE_STORAGE_BLOB_STORAGE_H__
#define AUTOMATON_CORE_STORAGE_BLOB_STORAGE_H__

#include <stdint.h>
#include <string>

/**
  blob_storage interface.
  Can store and delete arbitrary length data in a memory mapped file.
  There are no protections. The pointer points to the internal memory.
  The user of the storage should use the memory pointed by out_blob_pointer,
  up to out_blob_pointer + size. 
*/
class blob_storage {
 public:
  blob_storage(std::string file_path);
  ~blob_storage();
  /** 
    Creates a blob with a given size and returns the ID used to access it.

    @param[in]  size              The size in bytes to be allocated
    @param[out] out_blob_poiter   Pointer to the allocated memory
  */
  uint64_t create_blob(uint32_t size, uint8_t * out_blob_pointer);

  /** 
    Stores the data pointed by data, returns the ID used to access it.

    @param[in]  size      The size of the data pointed by data in bytes
    @param[out] data      Pointer to the data
  */
  uint64_t store_data(uint32_t size, uint8_t * data);


  /** 
    Used to get access to previously allocated blob. 
    Returns the size of the blob or 0 if id>=capacity

    @param[in]  id                The ID returned by create_blob
    @param[out] out_blob_poiter   Pointer to a previously created blob
  */
  uint32_t get_data(uint64_t id, uint8_t * out_blob_pointer);

  /** 
    Frees allocated blob. If there is no allocated blob with the given ID 
    the function returns false.

    @param[in]  id                The ID returned by create_blob
  */
  bool delete_blob(uint32_t id);
 private:
  uint32_t * storage;
  std::string file_path;
  uint64_t next_free;
  uint64_t capacity;
};

#endif  // AUTOMATON_CORE_STORAGE_BLOB_STORAGE_H__
