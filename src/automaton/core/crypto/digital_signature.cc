#include "automaton/core/crypto/digital_signature.h"

#include "automaton/core/data/msg.h"
#include "automaton/core/log/log.h"

using automaton::core::data::msg;

namespace automaton {
namespace core {
namespace crypto {

common::status digital_signature::process(const obj& request, obj* response) {
  const msg* request_msg = dynamic_cast<const msg*>(&request);
  auto request_msg_id = request_msg->get_schema_id();
  auto request_msg_type = request_msg->get_message_type();

  if (request_msg_type == "crypto.v0.dsig.sign.request") {
    auto private_key = request_msg->get_blob(1);
    auto message = request_msg->get_blob(2);
    uint8_t * signature = new uint8_t[signature_size()];
    if (response != nullptr) {
      msg* response_msg = dynamic_cast<msg*>(response);
      sign(reinterpret_cast<const uint8_t*>(private_key.c_str()),
           reinterpret_cast<const uint8_t*>(message.c_str()),
           message.size(),
           signature);
      response_msg->set_blob(1, std::string(reinterpret_cast<char*>(signature), signature_size()));
    }
    delete[] signature;
    return common::status::ok();
  }

  return common::status::unimplemented(request_msg_type + " not implemented!");
}

}  // namespace crypto
}  // namespace core
}  // namespace automaton
