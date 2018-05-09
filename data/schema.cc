#include "data/schema.h"

namespace data {

schema::field_info::field_info(int tag,
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

std::map<std::string, schema::factory_function_schema_def> schema::schema_definition_factory;

void schema::register_factory(std::string name, schema::factory_function_schema_def func) {
  schema_definition_factory[name] = func;
}

schema* schema::create(std::string name) {
  auto it = schema_definition_factory.find(name);
  if (it == schema_definition_factory.end()) {
    return nullptr;
  } else {
    return it->second();
  }
}

}  // namespace data
