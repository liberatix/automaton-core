#include "automaton/core/data/schema.h"

namespace automaton {
namespace core {
namespace data {

schema::field_info::field_info(uint32_t tag,
                               field_type type,
                               const std::string& name,
                               const std::string& fully_qualified_type,
                               bool is_repeated)
    : tag(tag)
    , type(type)
    , name(name)
    , fully_qualified_type(fully_qualified_type)
    , is_repeated(is_repeated) {
}

common::status schema::process(const obj& request, obj* response) {
  return common::status(common::status::OK);
}

}  // namespace data
}  // namespace core
}  // namespace automaton
