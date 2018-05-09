#ifndef AUTOMATON_CORE_DATA_FACTORY_H__
#define AUTOMATON_CORE_DATA_FACTORY_H__

#include <map>
#include <string>
#include <vector>
#include <utility>

#include "data/schema.h"
#include "data/msg.h"

/** Schema data structure interface.
*/
class factory {
 public:
  virtual ~factory() = 0;

  typedef factory* (*data_factory_function)();
  static void register_factory(std::string name, data_factory_function func);
  static factory* create(const std::string name);

  /**
    This is needed for testing or if virtual std::string serialize_protocol()
    exists.
  */
  virtual void import_schema_from_string(const std::string& protocol,
      const std::string& name, const std::string& package) = 0;

  /**
    This function is used for include schema definitions that were created with
    schema. If the given schema has dependencies, they
    must be imported first or exception will be thrown. Name will be used for
    reference in schema::add_dependency(). Package is used in
    fully_qualified_type.
  */
  virtual void import_schema_definition(schema* schema,
                                        const std::string& name,
                                        const std::string& package) = 0;

  /**
    Prints information about an enum and its values. If the given enum id is not
    valid, exception will be thrown.
  */
  virtual void dump_enum(int enum_id, std::ostream& ostream_) = 0;

  /**
    Returns the number of enums in the whole schema definition (both top-level
    and nested).
  */
  virtual int get_enums_number() = 0;

  /**
    Get the id of the enum with enum_name. Id is needed for getting information
    about the enum values. If enum with the given name doesn't exist, exception
    will be thrown.
  */
  virtual int get_enum_id(const std::string& enum_name) = 0;

  /**
    Returns enum value matching value name. If no such enum or name exists,
    exception will be thrown.
  */
  virtual int get_enum_value(int enum_id, const std::string& value_name) = 0;

  /**
    Returns a vector of pairs containing info about the values in this enum.
    A pair represents the string name of the value and the int value. If no such
    enum exists, exception will be thrown.
  */
  virtual std::vector<std::pair<std::string, int> >
      get_enum_values(int enum_id) = 0;

  /**
    Prints information about a message schema and its fields. If the given
    schema id is not valid, exception will be thrown.
  */
  virtual void dump_message_schema(int schema_id, std::ostream& ostream_) = 0;

  /**
    Returns the number of messages in the whole schema definition (both
    top-level and nested).
  */
  virtual int get_schemas_number() = 0;

  /**
    Returns the number of fields in the schema with the given id. If no such
    schema exists, exception will be thrown.
  */
  virtual int get_fields_number(int schema_id) = 0;

  /**
    Returns info about the specified field. Index must be >= 0 and <
    get_fields_number(schema_id). If no such schema or such field exists,
    exception will be thrown.
  */
  virtual schema::field_info get_field_info(int schema_id,
                                                       int index) = 0;

  /**
    Creates new message from a schema with message_type_id. Returns the
    created message that is used for setting and getting data,
    serializing and deserializing, etc.

    If the given message_type_id is not valid, exception will be thrown.
  */
  virtual msg* new_message(int message_type_id) = 0;

  /**
    Creates new message from a schema name.
    
    Returns the created message that is used for setting and getting data,
    serializing and deserializing, etc.

    If the given schema_name is not valid, exception will be thrown.
  */
  virtual msg* new_message(const char* schema_name) = 0;

  /** Returns the id of the schema with schema_name. Id is needed for creating
      new messages of that type, also getting information about the fields.
      If schema with the given name doesn't exist, exception will be thrown.
  */
  virtual int get_schema_id(const std::string& schema_name) = 0;

  /** Returns the name of the schema of message with the given id. If schema
      with the given name doesn't exist, exception will be thrown.
  */
  virtual std::string get_schema_name(int schema_id) = 0;

  // TODO(asen): We should return enum type instead of string.
  /** Returns the type (as a string) of the field with the given tag in the
      schema with the given id. If the given schema id is not valid or there is
      no field with the given tag, exception will be thrown.

      If the type is message, returns 'message'. To get the type of a message
      field use get_message_field_type().

      If the type is enum, returns 'enum'. To get the type of a enum field use
      get_enum_field_type().

      Use the result of this function in set_*() and get_*() and in
      set_repeated_*() and get_repeated_*() if the field is repeated.
  */
  virtual std::string get_field_type(int schema_id, int tag) = 0;

  /** If the type of the field is message, returns the fully-qualified name of
      the message type. If the given schema id is not valid or if there is no
      field with the given tag or the field type is not message,
      exception will be thrown.
  */
  virtual std::string get_message_field_type(int schema_id, int tag) = 0;

  /** If the type of the field is enum, returns the fully-qualified name of
      the enum type. If the given schema id is not valid or if there is no field
      with the given tag or the field type is not enum, exception will be thrown.
  */
  virtual std::string get_enum_field_type(int schema_id, int tag) = 0;

  /** Returns the tag of the field with the given name in the schema with the
      given id. If the given schema id is not valid or there is no field with the
      given name, exception will be thrown.
  */
  virtual int get_field_tag(int schema_id, const std::string& name) = 0;

  /** Returns true if the field is repeated, false, otherwise. If the given schema
      id is not valid or if there is no field with the given tag, exception will
      be thrown.
  */
  virtual bool is_repeated(int schema_id, int tag) = 0;

 private:
  static std::map<std::string, data_factory_function> schema_factory;
};

#endif  // AUTOMATON_CORE_DATA_FACTORY_H__