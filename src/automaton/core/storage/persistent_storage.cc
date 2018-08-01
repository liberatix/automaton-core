#include "automaton/core/storage/persistent_storage.h"
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/filesystem.hpp>
#include <string>

namespace automaton {
namespace core {
namespace storage {

persistent_storage::persistent_storage(size_t object_size)
    : next_free(0)
    , object_size(object_size)
    , capacity(0)
    , is_mapped(false) {
}

persistent_storage::~persistent_storage() {
  mmf.close();
}

uint8_t* persistent_storage::create_blob(const uint32_t size, uint64_t* id) {
  if (is_mapped == false) {
    throw std::logic_error("not mapped");
  }

  uint32_t size_in_int32 =  size % 4 ? size / 4 + 1 : size / 4;
  *id = next_free;
  // When the capacity is not large enough to store the required blob,
  // allocate a new memory block with double the size and copy the data.
  if (next_free + size_in_int32 >= capacity) {
    capacity *= 2;
    uint32_t * new_storage = new uint32_t[capacity];
    std::memcpy(new_storage, storage, capacity*sizeof(int32_t)*2);
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

bool persistent_storage::store(const uint64_t at, const uint8_t* data) {
  if (is_mapped == false) {
    throw std::logic_error("not mapped");;
  }
  // uint8_t* blob = create_blob(size, &id);

  // convert at to internal location
  // TODO(Samir): Consider just creating multiple files instead of keeping the data in one
  uint64_t location = at * object_size;
  if (location + object_size > capacity) {
    capacity *= 2;
    uint8_t * new_storage = new uint8_t[capacity];
    std::memcpy(new_storage, storage, capacity*sizeof(int32_t)*2);
    delete[] storage;
    storage = new_storage;
  }

  //std::memcpy(blob, data, size);
  return true;
}

uint8_t* persistent_storage::get(const uint64_t id) {
  if (is_mapped == false) {
    throw std::logic_error("not mapped");;
  }

  // check if id is out of range
  if (id+1 >= capacity) {
    return 0;
  }
  *size = storage[id];
  return reinterpret_cast<uint8_t*>(&storage[id + 1]);
}

bool persistent_storage::free(const uint32_t id) {
  // TODO(Samir): Change storage to unt8_t. Mark deleted nodes with *= -1
  storage[id] = 0;
  return 1;
}

bool persistent_storage::map_file(std::string path) {
  if (mmf.is_open()) {
    return false;
  }
  if (boost::filesystem::exists(path)) {
    mmf.open(path, boost::iostreams::mapped_file::mapmode::readwrite);
    storage = reinterpret_cast<uint32_t*>(mmf.data());
    capacity = mmf.size() / 4;
  } else {
    boost::iostreams::mapped_file_params new_mmf(path);
    new_mmf.flags = boost::iostreams::mapped_file::mapmode::readwrite;
    // The starting size is 1gb by default, we should add a option set the starting size
    new_mmf.new_file_size = 1ULL << 10;
    capacity = new_mmf.new_file_size / 4;
    mmf.open(new_mmf);
    storage = reinterpret_cast<uint32_t*>(mmf.data());
  }
  is_mapped = true;
  return true;
}

}  //  namespace storage
}  //  namespace core
}  //  namespace automaton
