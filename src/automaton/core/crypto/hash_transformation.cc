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

common::status hash_transformation::process(const obj& request, obj* response) {
  const msg* request_msg = dynamic_cast<const msg*>(&request);
  auto request_msg_id = request_msg->get_schema_id();
  auto request_msg_type = request_msg->get_message_type();

  // TODO(asen): This needs to be done using request_msg_id instead, but this needs the factory.
  if (request_msg_type == "crypto.v0.hash.digest.request") {
    std::string input = request_msg->get_blob(1);
    uint8_t* digest = new uint8_t[digest_size()];
    calculate_digest(reinterpret_cast<const uint8_t*>(input.c_str()), input.size(), digest);
    if (response != nullptr) {
      msg* response_msg = dynamic_cast<msg*>(response);
      response_msg->set_blob(1, std::string(reinterpret_cast<const char*>(digest), digest_size()));
    }
    delete[] digest;
    return common::status::ok();
  }

  if (request_msg_type == "crypto.v0.hash.size.request") {
    if (response != nullptr) {
      msg* response_msg = dynamic_cast<msg*>(response);
      response_msg->set_uint32(1, digest_size());
    }
    return common::status::ok();
  }

  if (request_msg_type == "crypto.v0.hash.update.request") {
    std::string input = request_msg->get_blob(1);
    update(reinterpret_cast<const uint8_t*>(input.c_str()), input.size());
    return common::status::ok();
  }

  if (request_msg_type == "crypto.v0.hash.final.request") {
    uint8_t* digest = new uint8_t[digest_size()];
    final(digest);
    if (response != nullptr) {
      msg* response_msg = dynamic_cast<msg*>(response);
      response_msg->set_blob(1, std::string(reinterpret_cast<const char*>(digest), digest_size()));
    }
    delete[] digest;
    return common::status::ok();
  }

  if (request_msg_type == "crypto.v0.hash.restart.request") {
    restart();
    return common::status::ok();
  }

  return common::status::unimplemented(request_msg_type + " not implemented!");
}

}  // namespace crypto
}  // namespace core
}  // namespace automaton
