#include "automaton/core/storage/persistent_storage.h"
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/filesystem.hpp>
#include <string>

#include <iostream>

namespace automaton {
namespace core {
namespace storage {

  persistent_storage::persistent_storage()
    : next_free(0)
    , capacity(0)
    , header_size(1024)
    , cur_version(10000)
    , is_mapped(false) {
}

persistent_storage::~persistent_storage() {
  mmf.close();
}



bool persistent_storage::store(const uint64_t at, const uint8_t* data) {
  if (is_mapped == false) {
    throw std::logic_error("not mapped");;
  }
  uint64_t location = at * object_size + header_size;
  // Increase capacity if necessary 
  while (location + object_size > capacity) {
    close_mapped_file();
    capacity *= 2;
    boost::filesystem::path boost_path(file_path);
    boost::filesystem::resize_file(boost_path, capacity);
    open_mapped_file();
  }
  memcpy(storage + location, data, object_size);
  return true;
}

uint8_t* persistent_storage::get(const uint64_t at) {
  if (is_mapped == false) {
    throw std::logic_error("not mapped");;
  }

  // check if id is out of range
  uint64_t location = at * object_size + header_size;
  if (location >= capacity) {
    throw std::out_of_range("index out of range");
  }
  //-- *size = storage[id];
  return reinterpret_cast<uint8_t*>(&storage[location]);
}

bool persistent_storage::free(const uint32_t id) {
  // TODO(Samir): Change storage to unt8_t. Mark deleted nodes with *= -1
  storage[id] = 0;
  return 1;
}

bool persistent_storage::map_file(std::string path, size_t object_sz) {
  if (mmf.is_open() || is_mapped) {
    return false;
  }
  file_path = path;
  object_size = object_sz;

  if (boost::filesystem::exists(file_path)) {
    mmf.open(file_path, boost::iostreams::mapped_file::mapmode::readwrite);
    storage = reinterpret_cast<uint8_t*>(mmf.data());
    capacity = mmf.size();

    //header_version = ((uint64_t*)storage)[0];
    memcpy(&header_version, storage, sizeof(uint64_t));
  } else {
    boost::iostreams::mapped_file_params new_mmf(file_path);
    new_mmf.flags = boost::iostreams::mapped_file::mapmode::readwrite;
    // The starting size is MB by default, we should add a option set the starting size
    new_mmf.new_file_size = 1ULL << 10;
    capacity = new_mmf.new_file_size;
    mmf.open(new_mmf);
    // write the version to the header of the new file
    storage = reinterpret_cast<uint8_t*>(mmf.data());
    uint64_t* header = reinterpret_cast<uint64_t*>(storage);
    header_version = cur_version;
    *header = header_version;
  }
  is_mapped = true;
  return true;
}

void persistent_storage::close_mapped_file() {
  mmf.close();
  is_mapped = false;
}

void persistent_storage::open_mapped_file() {
  mmf.open(file_path, boost::iostreams::mapped_file::mapmode::readwrite);
  storage = reinterpret_cast<uint8_t*>(mmf.data());
  capacity = mmf.size();
  is_mapped = true;
}

}  //  namespace storage
}  //  namespace core
}  //  namespace automaton
