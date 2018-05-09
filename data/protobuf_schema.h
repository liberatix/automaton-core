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

#include "data/factory.h"
#include "data/schema_definition.h"
#include "data/protobuf_schema_message.h"

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

class protobuf_schema: public factory {
 private:
  io_error_collector io_error_collector_;
  proto_error_collector proto_error_collector_;
  google::protobuf::DescriptorPool* pool;
  google::protobuf::DynamicMessageFactory* dynamic_message_factory = nullptr;

  /// Spots in the messages vector.
  std::stack<int> free_message_spots;

  /// Message type name to index in vector<const Message*> schemas
  std::map<std::string, int> schemas_names;

  /// Message schemas
  std::vector<const google::protobuf::Message*> schemas;

  // Message instances
  // std::vector <protobuf_schema_message> messages;

  /// Enum type name to index in vector enums
  std::map<std::string, int> enums_names;

  /// Enums
  std::vector<const google::protobuf::EnumDescriptor*> enums;

  /// Extracts schemas of nested messages
  void extract_nested_messages(const google::protobuf::Descriptor* descriptor);

  /// Extracts schemas of nested enums
  void extract_nested_enums(const google::protobuf::Descriptor* descriptor);

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
  msg* new_message(int schema_id);

  /**
    Creates new message from a schema name.
    
    Returns the created message that is used for setting and getting data,
    serializing and deserializing, etc.

    If the given schema_name is not valid, exception will be thrown.
  */
  msg* new_message(const char* schema_name);

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
};

#endif  // AUTOMATON_CORE_SCHEMA_PROTOBUF_SCHEMA_H_
