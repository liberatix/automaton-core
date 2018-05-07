#ifndef AUTOMATON_CORE_SCHEMA_PROTOBUF_SCHEMA_MESSAGE_H__
#define AUTOMATON_CORE_SCHEMA_PROTOBUF_SCHEMA_MESSAGE_H__

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

#include "schema/schema_message.h"

/**
  Google Protobuf schema_message implementation.
  
  This is a wrapper around google::protobuf::Message.
*/
class protobuf_schema_message : public schema_message {
 public:
  /**
    Constructs a protobuf schema_message implementation.
  */
  explicit protobuf_schema_message(google::protobuf::Message * m) : m(m) {}

  ~protobuf_schema_message() {
    if (m != NULL) {
      delete m;
    }
  }

  operator google::protobuf::Message*() {
    return m;
  }

  /**
    Returns the name of the message schema.

    If schema with the given name doesn't exist, exception will be thrown.
  */
  std::string get_message_type();

  /** Returns the size of the repeated field in the given message. If no such
      message or field exists or if the field is not repeated, exception will be
      thrown.
  */
  int get_repeated_field_size(int field_tag);

  /** Serializes the given message into the given string. If no such message
      exists or some error while serializing happens, exception will be thrown.
  */
  bool serialize_message(std::string* output);

  /** Deserializes message from the given string into a message with the given
      message id. If no such message exists or some error while deserializing
      happens, exception will be thrown. To use this function you should first
      create new message from the same schema and pass the id to this function.
  */
  bool deserialize_message(const std::string& input);

  /** Returns human readable representation of the message with the given id. If
      message id is not valid, exception will be thrown.
  */
  std::string to_string();

  /**
    Setters and getters for the scalar types. If any tag or id is invalid, or if
    you try to use these functions on non-matching type fields, or if you try to
    use get/set_repeated_*() on non-repeated field or vise versa, exception will
    be thrown. If you use out of range index on set_repeated_*() the value will
    just be added at the end (not on the specified index). If you use out of
    range index on set_repeated_*(), exception will be thrown.
  */
  void set_string(int field_tag, const std::string& value);

  std::string get_string(int field_tag);

  void set_repeated_string(int field_tag,
                                   const std::string& value,
                                   int index);
  std::string get_repeated_string(int field_tag, int index);

  void set_int32(int field_tag, int32_t value);

  int32_t get_int32(int field_tag);

  void set_repeated_int32(int field_tag, int32_t value, int index);

  int32_t get_repeated_int32(int field_tag, int index);

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
  void set_message(int field_tag,
                   const schema_message * sub_message);

  schema_message * get_message(int field_tag);

  void set_repeated_message(int field_tag,
                            const schema_message * sub_message,
                            int index);

  schema_message * get_repeated_message(int field_tag, int index);

  /*
    Setters and getters for enum type fields. If any tag or id is invalid, or if
    you try to use these functions on non-enum fields, exception will be thrown.
    If you use out of range index on set_repeated_enum() the value will just be
    added at the end (not on the specified index). If you use out of range index
    on get_repeated_enum(), exception will be thrown.
  */
  void set_enum(int field_tag, int value);

  int get_enum(int field_tag);

  void set_repeated_enum(int field_tag, int value, int index);

  int get_repeated_enum(int field_tag, int index);

 private:
  google::protobuf::Message* m;
};

#endif  // AUTOMATON_CORE_SCHEMA_PROTOBUF_SCHEMA_MESSAGE_H__
