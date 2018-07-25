#include "automaton/core/data/factory.h"

namespace automaton {
namespace core {
namespace data {

factory::~factory() {}

common::status factory::process(const obj& request, obj* response) {
  return common::status(common::status::OK);
}

}  // namespace data
}  // namespace core
}  // namespace automaton
