#include "automaton/core/data/protobuf/protobuf_msg.h"

#include <google/protobuf/util/json_util.h>

#include "automaton/core/log/log.h"

#ifdef GetMessage
#undef GetMessage
#endif  // !GetMessage

using std::string;

using google::protobuf::Descriptor;
using google::protobuf::EnumDescriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::Message;
using google::protobuf::Reflection;

using google::protobuf::util::MessageToJsonString;
using google::protobuf::util::JsonStringToMessage;

namespace automaton {
namespace core {
namespace data {
namespace protobuf {

string protobuf_msg::get_message_type() const {
  CHECK(m != nullptr);
  return m->GetTypeName();
}

unsigned int protobuf_msg::get_repeated_field_size(unsigned int field_tag) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  return m->GetReflection()->FieldSize(*m, fdesc);
}

bool protobuf_msg::serialize_message(string* output) const {
  CHECK_NOTNULL(output);
  CHECK_NOTNULL(m);
  return m->SerializeToString(output);  // TODO(kari): Handle errors.
}

bool protobuf_msg::deserialize_message(const string& input) {
  CHECK_NOTNULL(m);
  return m->ParseFromString(input);  // TODO(kari): Handle errors.
}

bool protobuf_msg::to_json(string* output) const {
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

string protobuf_msg::to_string() const {
  CHECK_NOTNULL(m);
  return m->DebugString();
}

void protobuf_msg::set_blob(unsigned int field_tag, const string& value) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->is_repeated()) {
    std::stringstream msg;
    msg << "Field is repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_STRING) {
    std::stringstream msg;
    msg << "Field is not blob!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  m->GetReflection()->SetString(m.get(), fdesc, value);
}

string protobuf_msg::get_blob(unsigned int field_tag) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->is_repeated()) {
    std::stringstream msg;
    msg << "Field is repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_STRING) {
    std::stringstream msg;
    msg << "Field is not blob!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  return m->GetReflection()->GetString(*m, fdesc);
}

void protobuf_msg::set_repeated_blob(unsigned int field_tag, const string& value, int index) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_STRING) {
    std::stringstream msg;
    msg << "Field is not blob!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    reflect->SetRepeatedString(m.get(), fdesc, index, value);
  } else {
    reflect->AddString(m.get(), fdesc, value);
  }
}

string protobuf_msg::get_repeated_blob(unsigned int field_tag, int index) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_STRING) {
    std::stringstream msg;
    msg << "Field is not blob!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    return reflect->GetRepeatedString(*m, fdesc, index);
  } else {
    std::stringstream msg;
    msg << "Index out of range: " << index;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::out_of_range(msg.str());
  }
}

/// Int 32

void protobuf_msg::set_int32(unsigned int field_tag, int32_t value) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->is_repeated()) {
    std::stringstream msg;
    msg << "Field is repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_INT32) {
    std::stringstream msg;
    msg << "Field is not int32!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  m->GetReflection()->SetInt32(m.get(), fdesc, value);
}

int32_t protobuf_msg::get_int32(unsigned int field_tag) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->is_repeated()) {
    std::stringstream msg;
    msg << "Field is repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_INT32) {
    std::stringstream msg;
    msg << "Field is not int32!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  return m->GetReflection()->GetInt32(*m, fdesc);
}

void protobuf_msg::set_repeated_int32(unsigned int field_tag, int32_t value, int index) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()
     ->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_INT32) {
    std::stringstream msg;
    msg << "Field is not int32!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    reflect->SetRepeatedInt32(m.get(), fdesc, index, value);
  } else {
    reflect->AddInt32(m.get(), fdesc, value);
  }
}

int32_t protobuf_msg::get_repeated_int32(unsigned int field_tag, int index) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_INT32) {
    std::stringstream msg;
    msg << "Field is not int32!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    return reflect->GetRepeatedInt32(*m, fdesc, index);
  } else {
    std::stringstream msg;
    msg << "Index out of range: " << index;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::out_of_range(msg.str());
  }
}

/// Unsigned int 32

void protobuf_msg::set_uint32(unsigned int field_tag, uint32_t value) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->is_repeated()) {
    std::stringstream msg;
    msg << "Field is repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_UINT32) {
    std::stringstream msg;
    msg << "Field is not uint32!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  m->GetReflection()->SetUInt32(m.get(), fdesc, value);
}

uint32_t protobuf_msg::get_uint32(unsigned int field_tag) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->is_repeated()) {
    std::stringstream msg;
    msg << "Field is repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_UINT32) {
    std::stringstream msg;
    msg << "Field is not uint32!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  return m->GetReflection()->GetUInt32(*m, fdesc);
}

void protobuf_msg::set_repeated_uint32(unsigned int field_tag, uint32_t value, int index) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()
     ->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_UINT32) {
    std::stringstream msg;
    msg << "Field is not uint32!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    reflect->SetRepeatedUInt32(m.get(), fdesc, index, value);
  } else {
    reflect->AddUInt32(m.get(), fdesc, value);
  }
}

uint32_t protobuf_msg::get_repeated_uint32(unsigned int field_tag, int index) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_UINT32) {
    std::stringstream msg;
    msg << "Field is not uint32!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    return reflect->GetRepeatedUInt32(*m, fdesc, index);
  } else {
    std::stringstream msg;
    msg << "Index out of range: " << index;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::out_of_range(msg.str());
  }
}

/// Int 64

void protobuf_msg::set_int64(unsigned int field_tag, int64_t value) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->is_repeated()) {
    std::stringstream msg;
    msg << "Field is repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_INT64) {
    std::stringstream msg;
    msg << "Field is not int64!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  m->GetReflection()->SetInt64(m.get(), fdesc, value);
}

int64_t protobuf_msg::get_int64(unsigned int field_tag) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->is_repeated()) {
    std::stringstream msg;
    msg << "Field is repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_INT64) {
    std::stringstream msg;
    msg << "Field is not int64!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  return m->GetReflection()->GetInt64(*m, fdesc);
}

void protobuf_msg::set_repeated_int64(unsigned int field_tag, int64_t value, int index) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()
     ->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_INT64) {
    std::stringstream msg;
    msg << "Field is not int64!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    reflect->SetRepeatedInt64(m.get(), fdesc, index, value);
  } else {
    reflect->AddInt64(m.get(), fdesc, value);
  }
}

int64_t protobuf_msg::get_repeated_int64(unsigned int field_tag, int index) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_INT64) {
    std::stringstream msg;
    msg << "Field is not int64!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    return reflect->GetRepeatedInt64(*m, fdesc, index);
  } else {
    std::stringstream msg;
    msg << "Index out of range: " << index;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::out_of_range(msg.str());
  }
}

/// Unsigned int 64

void protobuf_msg::set_uint64(unsigned int field_tag, uint64_t value) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->is_repeated()) {
    std::stringstream msg;
    msg << "Field is repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_UINT64) {
    std::stringstream msg;
    msg << "Field is not uint64!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  m->GetReflection()->SetUInt64(m.get(), fdesc, value);
}

uint64_t protobuf_msg::get_uint64(unsigned int field_tag) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->is_repeated()) {
    std::stringstream msg;
    msg << "Field is repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_UINT64) {
    std::stringstream msg;
    msg << "Field is not uint64!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  return m->GetReflection()->GetUInt64(*m, fdesc);
}

void protobuf_msg::set_repeated_uint64(unsigned int field_tag, uint64_t value, int index) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()
     ->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_UINT64) {
    std::stringstream msg;
    msg << "Field is not uint64!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    reflect->SetRepeatedUInt64(m.get(), fdesc, index, value);
  } else {
    reflect->AddUInt64(m.get(), fdesc, value);
  }
}

uint64_t protobuf_msg::get_repeated_uint64(unsigned int field_tag, int index) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_UINT64) {
    std::stringstream msg;
    msg << "Field is not uint64!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    return reflect->GetRepeatedUInt64(*m, fdesc, index);
  } else {
    std::stringstream msg;
    msg << "Index out of range: " << index;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::out_of_range(msg.str());
  }
}

/// Boolean

void protobuf_msg::set_boolean(unsigned int field_tag, bool value) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->is_repeated()) {
    std::stringstream msg;
    msg << "Field is repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_BOOL) {
    std::stringstream msg;
    msg << "Field is not boolean!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  m->GetReflection()->SetBool(m.get(), fdesc, value);
}

bool protobuf_msg::get_boolean(unsigned int field_tag) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->is_repeated()) {
    std::stringstream msg;
    msg << "Field is repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_BOOL) {
    std::stringstream msg;
    msg << "Field is not bool!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  return m->GetReflection()->GetBool(*m, fdesc);
}

void protobuf_msg::set_repeated_boolean(unsigned int field_tag, bool value, int index) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()
     ->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_BOOL) {
    std::stringstream msg;
    msg << "Field is not boolean!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    reflect->SetRepeatedBool(m.get(), fdesc, index, value);
  } else {
    reflect->AddBool(m.get(), fdesc, value);
  }
}

bool protobuf_msg::get_repeated_boolean(unsigned int field_tag, int index) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_BOOL) {
    std::stringstream msg;
    msg << "Field is not boolean!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    return reflect->GetRepeatedBool(*m, fdesc, index);
  } else {
    std::stringstream msg;
    msg << "Index out of range: " << index;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::out_of_range(msg.str());
  }
}


/// Message

void protobuf_msg::set_message(unsigned int field_tag, const msg& sub_message) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->is_repeated()) {
    std::stringstream msg;
    msg << "Field is repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) {
    std::stringstream msg;
    msg << "Field is not message!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  string message_type = fdesc->message_type()->full_name();  // Error
  auto& sub_m = dynamic_cast<const protobuf_msg&>(sub_message);
  string sub_message_type = sub_m.m->GetDescriptor()->full_name();
  if (message_type.compare(sub_message_type)) {
    std::stringstream msg;
    msg << "Type of the given sub message (which is <" << sub_message_type <<
        ">) doesn't match the field type (which is <" << message_type << ">)";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  // creates copy of the sub message so that the field doesnt change if you
  // change the message outside
  Message* copy = sub_m.m->New();
  copy->CopyFrom(*sub_m.m.get());
  reflect->SetAllocatedMessage(m.get(), copy, fdesc);
}

// makes a COPY of the message and returns its id
std::unique_ptr<msg> protobuf_msg::get_message(unsigned int field_tag) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->is_repeated()) {
    std::stringstream msg;
    msg << "Field is repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) {
    std::stringstream msg;
    msg << "Field is not message!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  // name resolution need to be done
  // to get Dynamic Message Factory of the class containing the sub message
  const Message* original = &reflect->GetMessage(*m, fdesc);
  Message* copy = original->New();
  copy->CopyFrom(*original);
  return std::unique_ptr<msg>(new protobuf_msg(copy));
}

void protobuf_msg::set_repeated_message(unsigned int field_tag, const msg& sub_message, int index) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) {
    std::stringstream msg;
    msg << "Field is not message!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  auto& sub_m = reinterpret_cast<const protobuf_msg&>(sub_message);
  if (sub_m.m.get() == nullptr || sub_m.m->GetDescriptor() == nullptr) {
    std::stringstream msg;
    msg << "No sub message!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::runtime_error(msg.str());
  }
  string message_type = fdesc->message_type()->full_name();  // Error
  string sub_message_type = sub_m.m->GetDescriptor()->full_name();
  if (message_type.compare(sub_message_type)) {
    std::stringstream msg;
    msg << "Type of the given sub message (which is <" << sub_message_type <<
        ">) doesn't match the field type (which is <" << message_type << ">)";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
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
std::unique_ptr<msg> protobuf_msg::get_repeated_message(unsigned int field_tag, int index) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) {
    std::stringstream msg;
    msg << "Field is not message!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    const Message* original = &reflect->GetRepeatedMessage(*m, fdesc, index);
    Message* copy = original->New();
    copy->CopyFrom(*original);
    return std::unique_ptr<msg>(new protobuf_msg(copy));
  } else {
    std::stringstream msg;
    msg << "Index out of range: " << index;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::out_of_range(msg.str());
  }
}

void protobuf_msg::set_enum(unsigned int field_tag, int value) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->is_repeated()) {
    std::stringstream msg;
    msg << "Field is repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_ENUM) {
    std::stringstream msg;
    msg << "Field is not enum!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const EnumDescriptor* edesc = fdesc->enum_type();
  if (edesc->FindValueByNumber(value) == nullptr) {
    std::stringstream msg;
    msg << "Enum doesn't have value: " << value;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  m->GetReflection()->SetEnumValue(m.get(), fdesc, value);
}

int protobuf_msg::get_enum(unsigned int field_tag) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc = m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->is_repeated()) {
    std::stringstream msg;
    msg << "Field is repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_ENUM) {
    std::stringstream msg;
    msg << "Field is not enum!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  return m->GetReflection()->GetEnumValue(*m, fdesc);
}

void protobuf_msg::set_repeated_enum(unsigned int field_tag, int value, int index) {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_ENUM) {
    std::stringstream msg;
    msg << "Field is not enum!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const EnumDescriptor* edesc = fdesc->enum_type();
  if (edesc->FindValueByNumber(value) == nullptr) {
    std::stringstream msg;
    msg << "Enum doesn't have value: " << value;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    reflect->SetRepeatedEnumValue(m.get(), fdesc, index, value);
  } else {
    reflect->AddEnumValue(m.get(), fdesc, value);
  }
}

int protobuf_msg::get_repeated_enum(unsigned int field_tag, int index) const {
  CHECK_NOTNULL(m);
  CHECK_NOTNULL(m->GetDescriptor());
  const FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    std::stringstream msg;
    msg << "No field with tag: " << field_tag;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_ENUM) {
    std::stringstream msg;
    msg << "Field is not enum!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  if (!(fdesc->is_repeated())) {
    std::stringstream msg;
    msg << "Field is not repeated!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::invalid_argument(msg.str());
  }
  const Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    return m->GetReflection()->GetRepeatedEnumValue(*m, fdesc, index);
  } else {
    std::stringstream msg;
    msg << "Index out of range: " << index;
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::out_of_range(msg.str());
  }
}

}  // namespace protobuf
}  // namespace data
}  // namespace core
}  // namespace automaton
