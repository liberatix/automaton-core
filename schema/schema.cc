#include "schema/schema.h"

schema::~schema() {}

std::map<std::string, schema::factory_function_schema> schema::schema_factory;

void schema::register_factory(std::string name, schema::factory_function_schema
    func) {
  schema_factory[name] = func;
}

schema* schema::create(std::string name) {
  auto it = schema_factory.find(name);
  if (it == schema_factory.end()) {
    return nullptr;
  } else {
    return it->second();
  }
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
