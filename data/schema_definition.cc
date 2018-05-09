#include "data/schema_definition.h"

schema_definition::field_info::field_info(
    int tag,
    field_type type,
    const std::string& name,
    const std::string& fully_qualified_type,
    bool is_repeated) {
  this->tag = tag;
  this->type = type;
  this->name = name;
  this->fully_qualified_type = fully_qualified_type;
  this->is_repeated = is_repeated;
}

std::map<std::string, schema_definition::factory_function_schema_def>
    schema_definition::schema_definition_factory;

void schema_definition::register_factory(std::string name,
    schema_definition::factory_function_schema_def func) {
  schema_definition_factory[name] = func;
}

schema_definition* schema_definition::create(std::string name) {
  auto it = schema_definition_factory.find(name);
  if (it == schema_definition_factory.end()) {
    return nullptr;
  } else {
    return it->second();
  }
}
