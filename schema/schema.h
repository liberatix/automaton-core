#ifndef SCHEMA_H_
#define SCHEMA_H_

#include <map>
#include <string>
#include <vector>
#include <utility>

#include "schema/schema_definition.h"

/*
  TODO(kari): how to make difference between schema_definition as a whole combo
  of message and enum schemas, and a single message/enum schema definition.
  It may be confusing.
  TODO(kari): field_info may become class. There are too many functions
  related to fields:
    * get_field_tag()
    * get_field_type_by_tag/name()
    * is_repeated()
    and others
*/

class schema {
 public:
  typedef schema* (*factory_function_schema)();
  static void register_factory(std::string name, factory_function_schema func);
  static schema* create(const std::string name);

  /*
    This is needed for testing or if virtual std::string serialize_protocol()
    exists.
  */
  virtual void import_schema_from_string(const std::string& protocol,
      const std::string& name, const std::string& package) = 0;

  /*
    This function is used for include schema definitions that were created with
    schema_definition. If the given schema_definition has dependencies, they
    must be imported first or exception will be thrown. Name will be used for
    reference in schema_definition::add_dependency(). Package is used in
    fully_qualified_type.
  */
  virtual void import_schema_definition(schema_definition* schema,
      const std::string& name, const std::string& package) = 0;

  /*
    Prints information about an enum and its values. If the given enum id is not
    valid, exception will be thrown.
  */
  virtual void dump_enum(int enum_id, std::ostream& ostream_) = 0;

  /*
    Returns the number of enums in the whole schema definition (both top-level
    and nested).
  */
  virtual int get_enums_number() = 0;

  /*
    Get the id of the enum with enum_name. Id is needed for getting information
    about the enum values. If enum with the given name doesn't exist, exception
    will be thrown.
  */
  virtual int get_enum_id(const std::string& enum_name) = 0;

  /*
    Returns enum value matching value name. If no such enum or name exists,
    exception will be thrown.
  */
  virtual int get_enum_value(int enum_id, const std::string& value_name) = 0;

  /*
    Returns a vector of pairs containing info about the values in this enum.
    A pair represents the string name of the value and the int value. If no such
    enum exists, exception will be thrown.
  */
  virtual std::vector<std::pair<std::string, int> >
      get_enum_values(int enum_id) = 0;

  /*
    Prints information about a message schema and its fields. If the given
    schema id is not valid, exception will be thrown.
  */
  virtual void dump_message_schema(int schema_id, std::ostream& ostream_) = 0;

  /*
    Returns the number of messages in the whole schema definition (both
    top-level and nested).
  */
  virtual int get_schemas_number() = 0;

  /*
    Returns the number of fields in the schema with the given id. If no such
    schema exists, exception will be thrown.
  */
  virtual int get_fields_number(int schema_id) = 0;

  /*
    Returns info about the specified field. Index must be >= 0 and <
    get_fields_number(schema_id). If no such schema or such field exists,
    exception will be thrown.
  */
  virtual schema_definition::field_info get_field_info(int schema_id,
                                                       int index) = 0;

  /*
    Creates new message from a schema with schema_id. Returns id of the
    created message that is used for setting and getting data,
    serializing and deserializing, etc. If the given schema id is not valid,
    exception will be thrown.
  */
  virtual int new_message(int schema_id) = 0;

  /*
    Creates a copy of the message with the given id. If the given id is not
    valid, exception will be thrown. Returns the id of the created copy.
  */
  virtual int create_copy_message(int message_id) = 0;

  /*
    Returns the id of the schema with schema_name. Id is needed for creating new
    messages of that type, also getting information about the fields. If schema
    with the given name doesn't exist, exception will be thrown.
  */
  virtual int get_schema_id(const std::string& schema_name) = 0;

  /*
    Returns the name of the schema of message with the given id. If schema with
    the given name doesn't exist, exception will be thrown.
  */
  virtual std::string get_schema_name(int schema_id) = 0;

  /*
    TODO(asen/kari): Decide to keep or remove this function. Removed for now.
    get_schema_name(get_message_schema_id(message_id)) will return the same
    information.

    Returns the schema name of the message with the given id. If such message
    doesn't exist, exception will be thrown.

  virtual std::string get_message_type(int message_id) = 0;
  */

  /*
    Returns the schema id of the message with the given id. If such message
    doesn't exist, exception will be thrown.
  */
  virtual int get_message_schema_id(int message_id) = 0;

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

  virtual std::string get_field_type(int schema_id, int tag) = 0;

  /*
    If the type of the field is message, returns the fully-qualified name of
    the message type. If the given schema id is not valid or if there is no
    field with the given tag or the field type is not message,
    exception will be thrown.
  */
  virtual std::string get_message_field_type(int schema_id, int tag) = 0;

  /*
    If the type of the field is enum, returns the fully-qualified name of
    the enum type. If the given schema id is not valid or if there is no field
    with the given tag or the field type is not enum, exception will be thrown.
  */
  virtual std::string get_enum_field_type(int schema_id, int tag) = 0;

  /*
    Returns the tag of the field with the given name in the schema with the
    given id. If the given schema id is not valid or there is no field with the
    given name, exception will be thrown.
  */
  virtual int get_field_tag(int schema_id, const std::string& name) = 0;

  /*
    Returns true if the field is repeated, false, otherwise. If the given schema
    id is not valid or if there is no field with the given tag, exception will
    be thrown.
  */
  virtual bool is_repeated(int schema_id, int tag) = 0;

  /*
    Returns the size of the repeated field in the given message. If no such
    message or field exists or if the field is not repeated, exception will be
    thrown.
  */
  virtual int get_repeated_field_size(int message_id, int field_tag) = 0;

  /*
    Serializes the given message into the given string. If no such message
    exists or some error while serializing happens, exception will be thrown.
  */
  virtual bool serialize_message(int message_id,  std::string* output) = 0;

  /*
    Deserializes message from the given string into a message with the given
    message id. If no such message exists or some error while deserializing
    happens, exception will be thrown. To use this function you should first
    create new message from the same schema and pass the id to this function.
  */
  virtual bool deserialize_message(int message_id, const std::string&
      input) = 0;

  /*
    Returns human readable representation of the message with the given id. If
    message id is not valid, exception will be thrown.
  */
  virtual std::string to_string(int message_id) = 0;

  /*
    Deletes the message with the given id. If message id is not valid, exception
    will be thrown.
  */
  virtual void delete_message(int message_id) = 0;

  /*
    Setters and getters for the scalar types. If any tag or id is invalid, or if
    you try to use these functions on non-matching type fields, or if you try to
    use get/set_repeated_*() on non-repeated field or vise versa, exception will
    be thrown. If you use out of range index on set_repeated_*() the value will
    just be added at the end (not on the specified index). If you use out of
    range index on set_repeated_*(), exception will be thrown.
  */

  virtual void set_string(int message_id, int field_tag,
      const std::string& value) = 0;
  virtual std::string get_string(int message_id, int field_tag) = 0;
  virtual void set_repeated_string(int message_id, int field_tag,
      const std::string& value, int index) = 0;
  virtual std::string get_repeated_string(int message_id, int field_tag,
      int index) = 0;

  virtual void set_int32(int message_id, int field_tag, int32_t value) = 0;
  virtual int32_t get_int32(int message_id, int field_tag) = 0;
  virtual void set_repeated_int32(int message_id, int field_tag, int32_t value,
      int index) = 0;
  virtual int32_t get_repeated_int32(int message_id, int field_tag,
      int index) = 0;

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
  virtual void set_message(int message_id, int field_tag,
      int sub_message_id) = 0;
  virtual int get_message(int message_id, int field_tag) = 0;
  virtual void set_repeated_message(int message_id, int field_tag, int
      sub_message_id, int index) = 0;
  virtual int get_repeated_message(int message_id, int field_tag,
      int index) = 0;

  /*
    Setters and getters for enum type fields. If any tag or id is invalid, or if
    you try to use these functions on non-enum fields, exception will be thrown.
    If you use out of range index on set_repeated_enum() the value will just be
    added at the end (not on the specified index). If you use out of range index
    on get_repeated_enum(), exception will be thrown.
  */
  virtual void set_enum(int message_id, int field_tag, int value) = 0;
  virtual int get_enum(int message_id, int field_tag) = 0;
  virtual void set_repeated_enum(int message_id, int field_tag, int value, int
      index) = 0;
  virtual int get_repeated_enum(int message_id, int field_tag, int index) = 0;

 private:
  static std::map<std::string, factory_function_schema> schema_factory;
};

#endif  // SCHEMA_H_
