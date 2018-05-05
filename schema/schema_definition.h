#ifndef AUTOMATON_CORE_SCHEMA_SCHEMA_DEFINITION_H__
#define AUTOMATON_CORE_SCHEMA_SCHEMA_DEFINITION_H__

#include <map>
#include <string>

class schema_definition {
 public:
  /**
   Allowed data types.
  **/
  enum field_type {
    unknown = 0,
    message_type =  1,
    enum_type = 2,
    string = 4,
    int32 = 5,
  };

  /**
   Struct representing one field in a message schema (field schema). Does NOT
   contain value, only describes a field.
   Tag - unique number within a message schema. Identifies the field.
   Type - one of the allowed data types. If it is a message or enum type,
     its message or enum type should be stored in fully_qualified_type.
   Name - unique name within a message schema. Identifies the field.
   Fully_qualified_type - specifies the type of message and enum fields. It is
     ignored for scalar types.
  */
  struct field_info {
    int tag;
    field_type type;
    std::string name;
    std::string fully_qualified_type;
    bool is_repeated;
    field_info(int tag, field_type type, const std::string& name,
        const std::string& fully_qualified_type, bool is_repeated);
  };

  typedef schema_definition* (*factory_function_schema_def)();
  static void register_factory(std::string name,
                               factory_function_schema_def func);
  static schema_definition* create(const std::string name);

  /**
    If this schema (schema A) depends on another one (schema B), it must be
    added here. Schema_name is the name that has been/will be given to the
    schema (B) on import (when schema::import_schema_definition() is called).
    Shema B should be imported before Schema A.
  **/
  virtual void add_dependency(const std::string& schema_name) = 0;

  /**
    Returns the id of the created message_schema.
    TODO(kari): Check for name collisions.
    Only creates empty message schema and name it. Fields and/or nested
    enums/messages should be added to the message (using add_* and
    add_*_field) and then add_message() so the created message schema is added
    to the schema.
  **/
  virtual int create_message(const std::string& message_name) = 0;

  /**
    Returns the id of the created enum_schema.
    TODO(kari): Check for name collisions.
    Only creates empty enum schema and name it. Values should be added to the
    enum (using add_enum_value) and then add_enum() so the created enum schema
    is added to the schema.
  **/
  virtual int create_enum(const std::string& enum_name) = 0;

  /**
    Used to add values to an already created enum with enum_id. If such enum
    doesn't exist, exception will be thrown.
    TODO(kari): Decide if duplicate values are allowed.
  **/
  virtual void add_enum_value(int enum_id, const std::string& value_name,
      int value) = 0;

  /**
    Used to add nested message. Both messages must already exist. If any of
    them doesn't exist, exception will be thrown. The message with id
    sub_message_id will be added as a nested message to the other.
    *** Nested message is different from message field. It is like an inner
    class, NOT like a reference to an object ***
    TODO(kari): Check for name collisions
    TODO(kari): Decide if duplicate values are allowed.
  **/
  virtual void add_nested_message(int message_id, int sub_message_id) = 0;

  /**
    Used to add an already created message/enum schema to this schema. The
    message/enum should first be created and ready (fields/ nested messages/
    enums or enum values must be added) before calling this function.
    Enum can be added to another message and if no message_id is provided in
    add_enum or message_id = -1, enum will be added globally. If message/enum
    with the given id doesn't exist, exception will be thrown.
  **/
  virtual void add_message(int message_id) = 0;
  virtual void add_enum(int enum_id, int message_id) = 0;

  /**
    These functions are called to add fields to a message. Any of them can be
    called multiple times on the same message. If the given message_id is not
    valid or the given field doesn't match the expected field type (ex.
    add_message_field() is called but the provided field is scalar type),
    exception will be thrown.
  **/
  virtual void add_scalar_field(field_info field, int message_id) = 0;
  virtual void add_enum_field(field_info field, int message_id) = 0;
  virtual void add_message_field(field_info field,
      int message_id) = 0;

 private:
  static std::map<std::string, factory_function_schema_def>
      schema_definition_factory;
};

#endif  // AUTOMATON_CORE_SCHEMA_SCHEMA_DEFINITION_H__
