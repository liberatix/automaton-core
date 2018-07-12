#include "automaton/core/storage/persistent_blobstore.h"
#include "automaton/core/storage/blobstore.h"
#include <cstring>

namespace automaton {
namespace core {
namespace storage {

persistent_blobstore::persistent_blobstore(std::string file_path)
    : file_path(file_path)
    , next_free(0)
    , capacity(0) {
  // TODO(Samir): Remove from constructor and do it when creating blob
  storage = new uint32_t[1ULL << 28];
  capacity = 1ULL << 28;
}

persistent_blobstore::~persistent_blobstore() {
  delete[] storage;
}

uint8_t* persistent_blobstore::create_blob(const uint32_t size, uint64_t* id) {
  uint32_t size_in_int32 =  size % 4 ? size / 4 + 1 : size / 4;
  *id = next_free;

  // When the capacity is not large enough to store the required blob,
  // allocate a new memory block with double the size and copy the data.
  if (next_free + size_in_int32 >= capacity) {
    capacity *= 2;
    uint32_t * new_storage = new uint32_t[capacity];
    std::memcpy(new_storage, storage, capacity*2);
    delete[] storage;
    storage = new_storage;
  }
  // Save the size of the blob
  storage[next_free] = size;
  // set the pointer to point to the blob
  uint8_t* out_blob_pointer = reinterpret_cast<uint8_t*>(&storage[next_free+1]);
  // next free equal to byte after the end of the blob
  next_free += size_in_int32 + 1;

  return out_blob_pointer;
}

uint64_t persistent_blobstore::store(const uint32_t size, const uint8_t* data) {
  uint64_t id = 0;
  uint8_t* blob = create_blob(size, &id);
  std::memcpy(blob, data, size);
  return id;
}

uint8_t* persistent_blobstore::get(const uint64_t id, uint32_t* size) {
  // check if id is out of range
  if (id+1 >= capacity) {
    return 0;
  }
  *size = storage[id];
  return reinterpret_cast<uint8_t*>(&storage[id + 1]);
}

bool persistent_blobstore::free(const uint32_t id) {
  // TODO(Samir): Change storage to unt8_t. Mark deleted nodes with *= -1
  storage[id] = 0;
  return 1;
}

}  //  namespace storage
}  //  namespace core
}  //  namespace automaton
