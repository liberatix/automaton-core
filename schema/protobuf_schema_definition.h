#ifndef AUTOMATON_CORE_PROTOBUF_SCHEMA_DEFINITION_H__
#define AUTOMATON_CORE_PROTOBUF_SCHEMA_DEFINITION_H__

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/message.h>
#include <google/protobuf/map.h>
#include <google/protobuf/compiler/parser.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/io/tokenizer.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <string>
#include <vector>

#include "schema/schema_definition.h"

class protobuf_schema_definition: public schema_definition {
 private:
  google::protobuf::Arena arena;
  google::protobuf::FileDescriptorProto* file_descriptor_proto;
  std::vector <google::protobuf::DescriptorProto*> messages;
  std::vector <google::protobuf::EnumDescriptorProto*> enums;

 public:
  protobuf_schema_definition();
  ~protobuf_schema_definition();

  void register_self();

  /**
    This function is used from protobuf_schema on import. (Instead of
    friend class)
  **/
  google::protobuf::FileDescriptorProto* get_descriptor();

  /**
    If this schema (schema A) depends on another one (schema B), it must be
    added here. Schema_name is the name that has been/will be given to the
    schema (B) on import (when schema::import_schema_definition() is called).
    Shema B should be imported before Schema A.
  **/
  void add_dependency(const std::string& schema_name);

  /**
    Returns the id of the created message_schema.
    TODO(kari): Check for name collisions.
    Only creates empty message schema and name it. Fields and/or nested
    enums/messages should be added to the message (using add_* and
    add_*_field) and then add_message() so the created message schema is added
    to the schema.
  **/
  int create_message(const std::string& message_name);

  /**
    Returns the id of the created enum_schema.
    TODO(kari): Check for name collisions.
    Only creates empty enum schema and name it. Values should be added to the
    enum (using add_enum_value) and then add_enum() so the created enum schema
    is added to the schema.
  **/
  int create_enum(const std::string& enum_name);

  /**
    Used to add values to an already created enum with enum_id. If such enum
    doesn't exist, exception will be thrown.
    TODO(kari): Decide if duplicate values are allowed.
  **/
  void add_enum_value(int enum_id, const std::string& value_name,
      int value);

  /**
    Used to add nested message. Both messages must already exist. If any of
    them doesn't exist, exception will be thrown. The message with id
    sub_message_id will be added as a nested message to the other.
    *** Nested message is different from message field. It is like an inner
    class, NOT like a reference to an object ***
    TODO(kari): Check for name collisions
    TODO(kari): Decide if duplicate values are allowed.
  **/
  void add_nested_message(int message_id, int sub_message_id);

  /**
    Used to add an already created message/enum schema to this schema. The
    message/enum should first be created and ready (fields/ nested messages/
    enums or enum values must be added) before calling this function.
    Enum can be added to another message and if no message_id is provided in
    add_enum or message_id = -1, enum will be added globally. If message/enum
    with the given id doesn't exist, exception will be thrown.
  **/
  void add_message(int message_id);
  void add_enum(int enum_id, int message_id);

  /**
    These functions are called to add fields to a message. Any of them can be
    called multiple times on the same message. If the given message_id is not
    valid or the given field doesn't match the expected field type (ex.
    add_message_field() is called but the provided field is scalar type),
    exception will be thrown.
  **/
  void add_scalar_field(schema_definition::field_info field, int message_id);
  void add_enum_field(schema_definition::field_info field, int message_id);
  void add_message_field(schema_definition::field_info field, int message_id);
};


#endif  // AUTOMATON_CORE_PROTOBUF_SCHEMA_DEFINITION_H__
