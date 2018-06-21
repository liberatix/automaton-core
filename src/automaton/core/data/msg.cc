#include "automaton/core/data/msg.h"

namespace automaton {
namespace core {
namespace data {

msg::~msg() {}

common::status msg::process(const obj& request, obj* response) {
  return common::status(common::OK);
}

}  // namespace data
}  // namespace core
}  // namespace automaton
