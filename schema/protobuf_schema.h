#ifndef AUTOMATON_CORE_SCHEMA_PROTOBUF_SCHEMA_H_
#define AUTOMATON_CORE_SCHEMA_PROTOBUF_SCHEMA_H_

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/message.h>
#include <google/protobuf/map.h>
#include <google/protobuf/compiler/parser.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/io/tokenizer.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <iostream>
#include <map>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "schema/schema.h"
#include "schema/schema_definition.h"

/**
This is helper class which is used while parsing proto file.
**/
class io_error_collector : public google::protobuf::io::ErrorCollector {
  int errors_number;
  std::string errors_list;
 public:
  io_error_collector();
  void AddError(int line, int column, const std::string& message);
  void AddWarning(int line, int column, const std::string& message);
  void clear_errors();
  int get_number_errors();
  std::string get_all_errors();
};
/**
This is helper class which is used while parsing proto file.
**/
class proto_error_collector : public
    google::protobuf::DescriptorPool::ErrorCollector {
  int errors_number;
  std::string errors_list;
 public:
  proto_error_collector();
  void AddError(
      const std::string& filename,  // File name in which the error occurred.
      const std::string& element_name,  // Full name of the erroneous element.
      const google::protobuf::Message* descriptor,  // Descriptor of the
          // erroneous element.
      google::protobuf::DescriptorPool::ErrorCollector::ErrorLocation location,
            // One of the location constants, above.
      const std::string& message);  // Human-readable error message.
  void AddWarning(
      const std::string& filename,  // File name in which the error occurred.
      const std::string& element_name,  // Full name of the erroneous element.
      const google::protobuf::Message* descriptor,  // Descriptor of the
          // erroneous element.
      google::protobuf::DescriptorPool::ErrorCollector::ErrorLocation location,
      // One of the location constants, above.
      const std::string& message);  // Human-readable error message.
  void clear_errors();
  int get_number_errors();
  std::string get_all_errors();
};

class protobuf_schema;
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

class protobuf_schema: public schema {
 private:
  io_error_collector io_error_collector_;
  proto_error_collector proto_error_collector_;
  google::protobuf::Arena arena;
  google::protobuf::DescriptorPool* pool;
  google::protobuf::DynamicMessageFactory* dynamic_message_factory = NULL;
  // Spots in the messages vector.
  std::stack<int> free_message_spots;
  // Message type name to index in vector<const Message*> schemas
  std::map<std::string, int> schemas_names;
  // Message schemas
  std::vector<const google::protobuf::Message*> schemas;
  // Message instances
  std::vector <google::protobuf::Message*> messages;

  // Enum type name to index in vector enums
  std::map<std::string, int> enums_names;
  // Enums
  std::vector<const google::protobuf::EnumDescriptor*> enums;

  /** Inserts a message in the message vector and returns its id. It is used
  when new messages are created. It inserts the message in a free spot in tne
  vector or adds it at the end of it
  **/
  int insert_message(google::protobuf::Message* message);
  // Extracts schemas of nested messages/enums
  void extract_nested_messages(
  const google::protobuf::Descriptor* descriptor);
  void extract_nested_enums(
      const google::protobuf::Descriptor* descriptor);

  /**
  Checks if there is protocol buffers data types that we do not support.
  Unsupported types are stored in the enum invalid_types.
  **/
  bool contain_invalid_data(const google::protobuf::Descriptor* descriptor);

  const google::protobuf::Message* get_schema(int schema_id);

  void import_from_file_proto(google::protobuf::FileDescriptorProto* fileproto,
      const std::string& name, const std::string& package);

 public:
  static const std::map<schema_definition::field_type,
      google::protobuf::FieldDescriptorProto_Type> type_to_protobuf_type;

  static const std::map<google::protobuf::FieldDescriptor::Type,
      schema_definition::field_type> protobuf_type_to_type;

  static const std::map<google::protobuf::FieldDescriptor::CppType,
      schema_definition::field_type> protobuf_ccptype_to_type;

  void register_self();
  /**
    TODO(kari): Decide to forbid move & copy constructors.
  **/
  protobuf_schema();
  ~protobuf_schema();

  /*
    This is needed for testing or if std::string serialize_protocol()
    exists.
  */
  void import_schema_from_string(const std::string& protocol,
      const std::string& name, const std::string& package);

  /*
    This function is used for include schema definitions that were created with
    schema_definition. If the given schema_definition has dependencies, they
    must be imported first or exception will be thrown. Name will be used for
    reference in schema_definition::add_dependency().
  */
  void import_schema_definition(schema_definition* schema,
      const std::string& name, const std::string& package);

  /**
    following functions are too complicated for mvp.
  **/
  // void import_data(data* data_, const std::string& name);
  // void import_message_to_schema(schema_definition* schema, int id, );
  // void import_enum_to_schema(schema_definition* schema, int id, );

  // std::string serialize_protocol();

  /*
    Prints information about an enum and its values. If the given enum id is not
    valid, exception will be thrown.
  */
  void dump_enum(int enum_id, std::ostream& ostream_);

  /*
    Returns the number of enums in the whole schema definition (both top-level
    and nested).
  */
  int get_enums_number();

  /*
    Get the id of the enum with enum_name. Id is needed for getting information
    about the enum values. If enum with the given name doesn't exist, exception
    will be thrown.
  */
  int get_enum_id(const std::string& enum_name);

  /*
    Returns enum value matching value name. If no such enum or name exists,
    exception will be thrown.
  */
  int get_enum_value(int enum_id, const std::string& value_name);

  /*
    Returns a vector of pairs containing info about the values in this enum.
    A pair represents the string name of the value and the int value. If no such
    enum exists, exception will be thrown.
  */
  std::vector<std::pair<std::string, int> > get_enum_values(int enum_id);

  /*
    Prints information about a message schema and its fields. If the given
    schema id is not valid, exception will be thrown.
  */
  void dump_message_schema(int schema_id, std::ostream& ostream_);

  /*
    Returns the number of messages in the whole schema definition (both
    top-level and nested).
  */
  int get_schemas_number();

  /*
    Returns the number of fields in the schema with the given id. If no such
    schema exists, exception will be thrown.
  */
  int get_fields_number(int schema_id);

  /*
    Returns info about the specified field. Index must be >= 0 and <
    get_fields_number(schema_id). If no such schema or such field exists,
    exception will be thrown.
  */
  schema_definition::field_info get_field_info(int schema_id, int index);

  /*
    Creates new message from a schema with schema_id. Returns id of the
    created message that is used for setting and getting data,
    serializing and deserializing, etc. If the given schema id is not valid,
    exception will be thrown.
  */
  int new_message(int schema_id);

  /*
    Creates a copy of the message with the given id. If the given id is not
    valid, exception will be thrown. Returns the id of the created copy.
  */
  int create_copy_message(int message_id);

  /*
    Returns the id of the schema with schema_name. Id is needed for creating new
    messages of that type, also getting information about the fields. If schema
    with the given name doesn't exist, exception will be thrown.
  */
  int get_schema_id(const std::string& schema_name);

  /*
    Returns the name of the schema of message with the given id. If schema with
    the given name doesn't exist, exception will be thrown.
  */
  std::string get_schema_name(int schema_id);

  /*
    Returns the schema id of the message with the given id. If such message
    doesn't exist, exception will be thrown.
  */
  int get_message_schema_id(int message_id);

  /*
    Returns the type (as a string) of the field with the given tag in the
    schema with the given id. If the given schema id is not valid or there is no
    field with the given tag, exception will be thrown.
    If the type is message, returns 'message'. To get the type of a message
    field use get_message_field_type().
    If the type is enum, returns 'enum'. To get the type of a enum field use
    get_enum_field_type().
    Use the result of this function in set_*() and get_*() and in
    set_repeated_*() and get_repeated_*() if the field is repeated.
  */
  std::string get_field_type(int schema_id, int tag);

  /*
    If the type of the field is message, returns the fully-qualified name of
    the message type. If the given schema id is not valid or if there is no
    field with the given tag or the field type is not message,
    exception will be thrown.
  */
  std::string get_message_field_type(int schema_id, int tag);

  /*
    If the type of the field is enum, returns the fully-qualified name of
    the enum type. If the given schema id is not valid or if there is no field
    with the given tag or the field type is not enum, exception will be thrown.
  */
  std::string get_enum_field_type(int schema_id, int tag);

  /*
    Returns the tag of the field with the given name in the schema with the
    given id. If the given schema id is not valid or there is no field with the
    given name, exception will be thrown.
  */
  int get_field_tag(int schema_id, const std::string& name);

  /*
    Returns true if the field is repeated, false, otherwise. If the given schema
    id is not valid or if there is no field with the given tag, exception will
    be thrown.
  */
  bool is_repeated(int schema_id, int tag);

  /*
    Returns the size of the repeated field in the given message. If no such
    message or field exists or if the field is not repeated, exception will be
    thrown.
  */
  int get_repeated_field_size(int message_id, int field_tag);

  /*
    Serializes the given message into the given string. If no such message
    exists or some error while serializing happens, exception will be thrown.
  */
  bool serialize_message(int message_id, std::string* output);


  /*
    Deserializes message from the given string into a message with the given
    message id. If no such message exists or some error while deserializing
    happens, exception will be thrown. To use this function you should first
    create new message from the same schema and pass the id to this function.
  */
  bool deserialize_message(int message_id, const std::string&
      input);

  /*
    Returns human readable representation of the message with the given id. If
    message id is not valid, exception will be thrown.
  */
  std::string to_string(int message_id);

  /*
    Deletes the message with the given id. If message id is not valid, exception
    will be thrown.
  */
  void delete_message(int message_id);

  /*
    Setters and getters for the scalar types. If any tag or id is invalid, or if
    you try to use these functions on non-matching type fields, or if you try to
    use get/set_repeated_*() on non-repeated field or vise versa, exception will
    be thrown. If you use out of range index on set_repeated_*() the value will
    just be added at the end (not on the specified index). If you use out of
    range index on get_repeated_*(), exception will be thrown.
  */

  void set_string(int message_id, int field_tag, const std::string& value);
  std::string get_string(int message_id, int field_tag);
  void set_repeated_string(int message_id, int field_tag, const std::string&
      value, int index);
  std::string get_repeated_string(int message_id, int field_tag,
      int index);

  void set_int32(int message_id, int field_tag, int32_t value);
  int32_t get_int32(int message_id, int field_tag);
  void set_repeated_int32(int message_id, int field_tag, int32_t value,
    int index);
  int32_t get_repeated_int32(int message_id, int field_tag, int index);

  /*
    Setters and getters for message type fields. When you use setters, you
    should already have the sub_message created and initialized. This function
    makes a copy of it so if you change the sub_message, the original message
    that keeps the copy will not be affected. If any tag or id is invalid, or if
    you try to use these functions on non-message fields, or if you try to use
    get/set_repeated_message() on non-repeated field or vise versa, or if you
    give sub_message that is from different type than expected, exception will
    be thrown. If you use out of range index on set_repeated_message() the value
    will just be added at the end (not on the specified index). If you use out
    of range index on get_repeated_message(), exception will be thrown.
  */
  void set_message(int message_id, int field_tag, int sub_message_id);
  int get_message(int message_id, int field_tag);
  void set_repeated_message(int message_id, int field_tag, int sub_message_id,
      int index);
  int get_repeated_message(int message_id, int field_tag, int index);

  /*
    Setters and getters for enum type fields. If any tag or id is invalid, or if
    you try to use these functions on non-enum fields, exception will be thrown.
    If you use out of range index on set_repeated_enum() the value will just be
    added at the end (not on the specified index). If you use out of range index
    on get_repeated_enum(), exception will be thrown.
  */
  void set_enum(int message_id, int field_tag, int value);
  int get_enum(int message_id, int field_tag);
  void set_repeated_enum(int message_id, int field_tag, int value, int
      index);
  int get_repeated_enum(int message_id, int field_tag, int index);
};

#endif  // AUTOMATON_CORE_SCHEMA_PROTOBUF_SCHEMA_H_
