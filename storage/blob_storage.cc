#include "storage/blob_storage.h"

blob_storage::blob_storage(std::string file_path)
    : file_path(file_path)
    , next_free(0)
    , capacity(0) {
  // TODO(Samir): Remove from constructor and do it when creating blob
  storage = new uint32_t[1ULL << 28];
  capacity = 1ULL << 30;
}

blob_storage::~blob_storage() {
  delete[] storage;
}

uint64_t blob_storage::create_blob(uint32_t size, uint8_t * out_blob_pointer) {
  // save next_free for later recording
  uint64_t accesss_id = next_free;

  // Convert from size in bytes to size in ints
  size % 4 ? size /= 4, size++ : size /= 4;

  // When the capacity is not large enough to store the required blob,
  // allocate a new memory block with double the size and copy the data.
  if (next_free + size >= capacity) {
    capacity *= 2;
    uint32_t * new_storage = new uint32_t[capacity];
    memcpy(new_storage, storage, capacity / 2);
    delete[] storage;
    storage = new_storage;
  }
  // Save the size of the blob
  storage[next_free] = size;
  // set the pointer to point to the blob
  out_blob_pointer = reinterpret_cast<uint8_t*>(&storage[next_free+1]);
  // next free equal to byte after the end of the blob
  next_free += size + 1;

  return accesss_id;
}

uint64_t blob_storage::store_data(uint32_t size, uint8_t * data) {
  uint8_t * blob = nullptr;
  create_blob(size, blob);
  memcpy(blob, data, size);
}

uint32_t blob_storage::get_data(uint64_t id, uint8_t * out_blob_pointer) {
  // check if id is out of range
  if (id+1 >= capacity) {
    return 0;
  }
  // set the pointer to point to the blob
  out_blob_pointer = reinterpret_cast<uint8_t*>(storage[id + 1]);
  // return the size of the blob
  return storage[id];
}

bool blob_storage::delete_blob(uint32_t id) {
  // TODO(Samir): implement delete, maybe use linked
  // list to track free memory locations
  throw;
}
