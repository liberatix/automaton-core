#include "schema/schema_definition.h"

schema_definition::field_info::field_info(int tag,
    field_type type, const std::string& name,
    const std::string& fully_qualified_type, bool is_repeated) {
  this->tag = tag;
  this->type = type;
  this->name = name;
  this->fully_qualified_type = fully_qualified_type;
  this->is_repeated = is_repeated;
}
