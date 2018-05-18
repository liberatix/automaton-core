#include "data/protobuf/protobuf_msg.h"

#include <google/protobuf/util/json_util.h>

#include "log/log.h"

using std::string;

using google::protobuf::Descriptor;
using google::protobuf::EnumDescriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::Message;
using google::protobuf::Reflection;

using google::protobuf::util::MessageToJsonString;
using google::protobuf::util::JsonStringToMessage;

namespace data {
namespace protobuf {

string protobuf_msg::get_message_type() const {
  CHECK(m != nullptr);
  return m->GetTypeName();
}

int protobuf_msg::get_repeated_field_size(int field_tag) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " + std::to_string(field_tag));
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  return m->GetReflection()->FieldSize(*m, fdesc);
}

bool protobuf_msg::serialize_message(string* output) {
  CHECK_NOTNULL(output);
  CHECK_NOTNULL(m);
  return m->SerializeToString(output);  // TODO(kari): Handle errors.
}

bool protobuf_msg::deserialize_message(const string& input) {
  CHECK_NOTNULL(m);
  return m->ParseFromString(input);  // TODO(kari): Handle errors.
}

bool protobuf_msg::to_json(string* output) {
  CHECK_NOTNULL(output);
  CHECK_NOTNULL(m);
  auto status = MessageToJsonString(*m, output);
  if (!status.ok()) {
    // TODO(asen): Needs better error handling
    std::cout << status.error_message() << std::endl;
  }
  return status.ok();
}

bool protobuf_msg::from_json(const string& input) {
  CHECK_NOTNULL(m);
  auto status = JsonStringToMessage(input, m.get());
  if (!status.ok()) {
    // TODO(asen): Needs better error handling
    std::cout << status.error_message() << std::endl;
  }
  return status.ok();
}

string protobuf_msg::to_string() {
  CHECK_NOTNULL(m);
  return m->DebugString();
}

void protobuf_msg::set_string(int field_tag, const string& value) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_STRING) {
    throw std::invalid_argument("Field is not string");
  }
  m->GetReflection()->SetString(m.get(), fdesc, value);
}

string protobuf_msg::get_string(int field_tag) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " + std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_STRING) {
    throw std::invalid_argument("Field is not string");
  }
  return m->GetReflection()->GetString(*m, fdesc);
}

void protobuf_msg::set_repeated_string(int field_tag,
      const string& value, int index = -1) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " + std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_STRING) {
    throw std::invalid_argument("Field is not string");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    reflect->SetRepeatedString(m.get(), fdesc, index, value);
  } else {
    reflect->AddString(m.get(), fdesc, value);
  }
}

string protobuf_msg::get_repeated_string(int field_tag, int index) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " + std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_STRING) {
    throw std::invalid_argument("Field is not string");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    return reflect->GetRepeatedString(*m, fdesc, index);
  } else {
    throw std::out_of_range("Index out of range: " + std::to_string(index));
  }
}

void protobuf_msg::set_int32(int field_tag, int32_t value) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " + std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_INT32) {
    throw std::invalid_argument("Field is not int32");
  }
  m->GetReflection()->SetInt32(m.get(), fdesc, value);
}

int32_t protobuf_msg::get_int32(int field_tag) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " + std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_INT32) {
    throw std::invalid_argument("Field is not int32");
  }
  return m->GetReflection()->GetInt32(*m, fdesc);
}

void protobuf_msg::set_repeated_int32(
    int field_tag, int32_t value, int index) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()
     ->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_INT32) {
    throw std::invalid_argument("Field is not int32");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    reflect->SetRepeatedInt32(m.get(), fdesc, index, value);
  } else {
    reflect->AddInt32(m.get(), fdesc, value);
  }
}

int32_t protobuf_msg::get_repeated_int32(int field_tag, int index) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_INT32) {
    throw std::invalid_argument("Field is not int32");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    return reflect->GetRepeatedInt32(*m, fdesc, index);
  } else {
    throw std::out_of_range("Index out of range: " + std::to_string(index));
  }
}

void protobuf_msg::set_message(int field_tag, const msg& sub_message) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " + std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) {
    throw std::invalid_argument("Field is not message");
  }
  string message_type = fdesc->message_type()->full_name();  // Error
  auto& sub_m = reinterpret_cast<const protobuf_msg&>(sub_message);
  string sub_message_type = sub_m.m->GetDescriptor()->full_name();
  if (message_type.compare(sub_message_type)) {
    throw std::invalid_argument("Type of the given sub message (which is <" +
        sub_message_type + ">) doesn't match the field type (which is <" +
        message_type + ">)");
  }
  const Reflection* reflect = m->GetReflection();
  // creates copy of the sub message so that the field doesnt change if you
  // change the message outside
  Message* copy = sub_m.m->New();
  copy->CopyFrom(*sub_m.m.get());
  reflect->SetAllocatedMessage(m.get(), copy, fdesc);
}

// makes a COPY of the message and returns its id
std::unique_ptr<msg> protobuf_msg::get_message(int field_tag) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  const Reflection* reflect = m->GetReflection();
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) {
    throw std::invalid_argument("Field is not message");
  }
  // name resolution need to be done
  // to get Dynamic Message Factory of the class containing the sub message
  const Message* original = &reflect->GetMessage(*m, fdesc);
  Message* copy = original->New();
  copy->CopyFrom(*original);
  return std::unique_ptr<msg>(new protobuf_msg(copy));
}

void protobuf_msg::set_repeated_message(int field_tag, const msg& sub_message, int index) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) {
    throw std::invalid_argument("Field is not message");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  auto& sub_m = reinterpret_cast<const protobuf_msg&>(sub_message);
  if (sub_m.m.get() == nullptr || sub_m.m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  string message_type = fdesc->message_type()->full_name();  // Error
  string sub_message_type = sub_m.m->GetDescriptor()->full_name();
  if (message_type.compare(sub_message_type)) {
    throw std::invalid_argument("Type of the given sub message (which is <" +
        sub_message_type + ">) doesn't match the field type (which is <" +
        message_type + ">)");
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    reflect->MutableRepeatedMessage(m.get(), fdesc, index)->CopyFrom(*sub_m.m.get());
  } else {
    Message* copy = sub_m.m->New();
    copy->CopyFrom(*sub_m.m.get());
    reflect->AddAllocatedMessage(m.get(), fdesc, copy);
  }
}

// Returns copy of the message
std::unique_ptr<msg> protobuf_msg::get_repeated_message(int field_tag, int index) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) {
    throw std::invalid_argument("Field is not message");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    const Message* original = &reflect->GetRepeatedMessage(*m, fdesc, index);
    Message* copy = original->New();
    copy->CopyFrom(*original);
    return std::unique_ptr<msg>(new protobuf_msg(copy));
  } else {
    throw std::out_of_range("Index out of range: " + std::to_string(index));
  }
}

void protobuf_msg::set_enum(int field_tag, int value) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " + std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_ENUM) {
    throw std::invalid_argument("Field is not enum");
  }
  const EnumDescriptor* edesc = fdesc->enum_type();
  if (edesc->FindValueByNumber(value) == nullptr) {
    throw std::invalid_argument("Enum doesn't have value: " + std::to_string(value));
  }
  m->GetReflection()->SetEnumValue(m.get(), fdesc, value);
}

int protobuf_msg::get_enum(int field_tag) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " + std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_ENUM) {
    throw std::invalid_argument("Field is not enum");
  }
  return m->GetReflection()->GetEnumValue(*m, fdesc);
}

void protobuf_msg::set_repeated_enum(int field_tag, int value, int index = -1) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_ENUM) {
    throw std::invalid_argument("Field is not enum");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  const EnumDescriptor* edesc = fdesc->enum_type();
  if (edesc->FindValueByNumber(value) == nullptr) {
    throw std::invalid_argument("Enum doesn't have value: " + std::to_string(value));
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    reflect->SetRepeatedEnumValue(m.get(), fdesc, index, value);
  } else {
    reflect->AddEnumValue(m.get(), fdesc, value);
  }
}

int protobuf_msg::get_repeated_enum(int field_tag, int index) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_ENUM) {
    throw std::invalid_argument("Field is not enum");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    return m->GetReflection()->GetRepeatedEnumValue(*m, fdesc, index);
  } else {
    throw std::out_of_range("Index out of range: " + std::to_string(index));
  }
}

}  // namespace protobuf
}  // namespace data
