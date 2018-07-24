#include "automaton/core/crypto/secure_random.h"

#include "automaton/core/data/msg.h"
#include "automaton/core/log/log.h"

using automaton::core::data::msg;

namespace automaton {
namespace core {
namespace crypto {

common::status secure_random::process(const obj& request, obj* response) {
  const msg* request_msg = dynamic_cast<const msg*>(&request);
  auto request_msg_id = request_msg->get_schema_id();
  auto request_msg_type = request_msg->get_message_type();

  if (request_msg_type == "crypto.v0.rand.bit.request") {
    if (response != nullptr) {
      msg* response_msg = dynamic_cast<msg*>(response);
      response_msg->set_boolean(1, bit());
    }
    return common::status::ok();
  }

  if (request_msg_type == "crypto.v0.rand.byte.request") {
    if (response != nullptr) {
      msg* response_msg = dynamic_cast<msg*>(response);
      response_msg->set_uint32(1, byte());
    }
    return common::status::ok();
  }

  if (request_msg_type == "crypto.v0.rand.block.request") {
    uint32_t requested_size = request_msg->get_uint32(1);
    uint8_t * memblock = new uint8_t[requested_size];
    if (response != nullptr) {
      msg* response_msg = dynamic_cast<msg*>(response);
      block(memblock, requested_size);
      response_msg->set_blob(1, std::string(reinterpret_cast<char*>(memblock), requested_size));
    }
    delete[] memblock;
    return common::status::ok();
  }

  return common::status::unimplemented(request_msg_type + " not implemented!");
}

}  // namespace crypto
}  // namespace core
}  // namespace automaton
