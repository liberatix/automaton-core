#include "automaton/core/crypto/hash_transformation.h"

#include <string>

#include "automaton/core/data/msg.h"
#include "automaton/core/log/log.h"

using automaton::core::data::msg;

namespace automaton {
namespace core {
namespace crypto {

void hash_transformation::calculate_digest(const uint8_t * input,
                                           const size_t length,
                                           uint8_t * digest) {
  update(input, length);
  final(digest);
}

std::map<std::string, hash_transformation::factory_function_type>
    hash_transformation::hash_transformation_factory;

void hash_transformation::register_factory(std::string name,
                                           factory_function_type func) {
  // TODO(samir): decide what to do if function is already registered
  // auto it = hash_transformation_factory.find(name);
  // if (it != hash_transformation_factory.end()) {
  // }

  hash_transformation_factory[name] = func;
}

hash_transformation * hash_transformation::create(std::string name) {
  auto it = hash_transformation_factory.find(name);
  if (it == hash_transformation_factory.end()) {
    return nullptr;
  } else {
    return it->second();
  }
}

common::status hash_transformation::process(const obj& request, obj* response) {
  const msg* request_msg = dynamic_cast<const msg*>(&request);
  auto request_msg_id = request_msg->get_schema_id();
  auto request_msg_type = request_msg->get_message_type();

  // TODO(asen): This needs to be done using request_msg_id instead, but this needs the factory.
  if (request_msg_type == "crypto.v0.hash.calculate_digest.request") {
    std::string input = request_msg->get_blob(1);
    uint8_t* digest = new uint8_t[digest_size()];
    calculate_digest(reinterpret_cast<const uint8_t*>(input.c_str()), input.size(), digest);
    if (response != nullptr) {
      msg* response_msg = dynamic_cast<msg*>(response);
      response_msg->set_blob(1, std::string(reinterpret_cast<const char*>(digest), digest_size()));
    }
    delete[] digest;
  }

  if (request_msg_type == "crypto.v0.hash.update.request") {
    std::string input = request_msg->get_blob(1);
    update(reinterpret_cast<const uint8_t*>(input.c_str()), input.size());
  }

  if (request_msg_type == "crypto.v0.hash.final.request") {
    uint8_t* digest = new uint8_t[digest_size()];
    final(digest);
    if (response != nullptr) {
      msg* response_msg = dynamic_cast<msg*>(response);
      response_msg->set_blob(1, std::string(reinterpret_cast<const char*>(digest), digest_size()));
    }
    delete[] digest;
  }

  if (request_msg_type == "crypto.v0.hash.restart.request") {
    restart();
  }

  return common::status(common::status::OK);
}

}  // namespace crypto
}  // namespace core
}  // namespace automaton
