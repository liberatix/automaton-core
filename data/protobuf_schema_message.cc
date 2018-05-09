#include "data/protobuf_schema_message.h"

std::string protobuf_schema_message::get_message_type() {
  if (m == nullptr) {
    throw std::runtime_error("Unexpected error: No message");
  }
  return m->GetTypeName();
}

int protobuf_schema_message::get_repeated_field_size(int field_tag) {
  if (m == nullptr || m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  return m->GetReflection()->FieldSize(*m, fdesc);
}

bool protobuf_schema_message::serialize_message(std::string* output) {
  if (output == nullptr) {
    throw std::invalid_argument("No output provided");
  }
  if (m == nullptr) {
    throw std::runtime_error("Unexpected error: No message");
  }
  return m->SerializeToString(output);  // TODO(kari): Handle errors.
}

bool protobuf_schema_message::deserialize_message(const std::string& input) {
  if (m == nullptr) {
    throw std::runtime_error("Unexpected error: No message");
  }
  return m->ParseFromString(input);  // TODO(kari): Handle errors.
}

std::string protobuf_schema_message::to_string() {
  if (m == nullptr) {
    throw std::runtime_error("Unexpected error: No message");
  }
  return m->DebugString();
}

void protobuf_schema_message::set_string(int field_tag,
    const std::string& value) {
  if (m == nullptr || m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error: No message");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);

  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_STRING) {
    throw std::invalid_argument("Field is not string");
  }
  m->GetReflection()->SetString(m, fdesc, value);
}

std::string protobuf_schema_message::get_string(int field_tag) {
  if (m == nullptr || m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_STRING) {
    throw std::invalid_argument("Field is not string");
  }
  return m->GetReflection()->GetString(*m, fdesc);
}

void protobuf_schema_message::set_repeated_string(int field_tag,
      const std::string& value, int index = -1) {
  if (m == nullptr || m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_STRING) {
    throw std::invalid_argument("Field is not string");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  const google::protobuf::Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    reflect->SetRepeatedString(m, fdesc, index, value);
  } else {
    reflect->AddString(m, fdesc, value);
  }
}

std::string protobuf_schema_message::get_repeated_string(
      int field_tag, int index) {
  if (m == nullptr || m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_STRING) {
    throw std::invalid_argument("Field is not string");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  const google::protobuf::Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    return reflect->GetRepeatedString(*m, fdesc, index);
  } else {
    throw std::out_of_range("Index out of range: " + std::to_string(index));
  }
}

void protobuf_schema_message::set_int32(int field_tag, int32_t value) {
  if (m == nullptr || m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_INT32) {
    throw std::invalid_argument("Field is not int32");
  }
  m->GetReflection()->SetInt32(m, fdesc, value);
}

int32_t protobuf_schema_message::get_int32(int field_tag) {
  if (m == nullptr || m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
     ->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_INT32) {
    throw std::invalid_argument("Field is not int32");
  }
  return m->GetReflection()->GetInt32(*m, fdesc);
}

void protobuf_schema_message::set_repeated_int32(
    int field_tag, int32_t value, int index) {
  if (m == nullptr || m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
     ->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_INT32) {
    throw std::invalid_argument("Field is not int32");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  const google::protobuf::Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    reflect->SetRepeatedInt32(m, fdesc, index, value);
  } else {
    reflect->AddInt32(m, fdesc, value);
  }
}

int32_t protobuf_schema_message::get_repeated_int32(int field_tag, int index) {
  if (m == nullptr || m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_INT32) {
    throw std::invalid_argument("Field is not int32");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  const google::protobuf::Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    return reflect->GetRepeatedInt32(*m, fdesc, index);
  } else {
    throw std::out_of_range("Index out of range: " + std::to_string(index));
  }
}

void protobuf_schema_message::set_message(
    int field_tag, const msg* sub_message) {
  if (m == nullptr || m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error: No message");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
    throw std::invalid_argument("Field is not message");
  }
  std::string message_type = fdesc->message_type()->full_name();  // Error
  google::protobuf::Message* sub_m =
      reinterpret_cast<const protobuf_schema_message *>(sub_message)->m;
  std::string sub_message_type = sub_m->GetDescriptor()->full_name();
  if (message_type.compare(sub_message_type)) {
    throw std::invalid_argument("Type of the given sub message (which is <" +
        sub_message_type + ">) doesn't match the field type (which is <" +
        message_type + ">)");
  }
  const google::protobuf::Reflection* reflect = m->GetReflection();
  // creates copy of the sub message so that the field doesnt change if you
  // change the message outside
  google::protobuf::Message* copy = sub_m->New();
  copy->CopyFrom(*sub_m);
  reflect->SetAllocatedMessage(m, copy, fdesc);
}

// makes a COPY of the message and returns its id
msg * protobuf_schema_message::get_message(int field_tag) {
  if (m == nullptr || m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  const google::protobuf::Reflection* reflect = m->GetReflection();
    if (fdesc->cpp_type() !=
        google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
    throw std::invalid_argument("Field is not message");
  }
  // name resolution need to be done
  // to get Dynamic Message Factory of the class containing the sub message
  const google::protobuf::Message* original = &reflect->GetMessage(*m, fdesc);
  google::protobuf::Message* copy = original->New();
  copy->CopyFrom(*original);
  return new protobuf_schema_message(copy);
}

void protobuf_schema_message::set_repeated_message(
    int field_tag, const msg * sub_message, int index = -1) {
  if (m == nullptr || m->GetDescriptor() == nullptr || sub_message == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
    throw std::invalid_argument("Field is not message");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  google::protobuf::Message* sub_m =
      reinterpret_cast<const protobuf_schema_message *>(sub_message)->m;
  if (sub_m == nullptr || sub_m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  std::string message_type = fdesc->message_type()->full_name();  // Error
  std::string sub_message_type =
      sub_m->GetDescriptor()->full_name();
  if (message_type.compare(sub_message_type)) {
    throw std::invalid_argument("Type of the given sub message (which is <" +
        sub_message_type + ">) doesn't match the field type (which is <" +
        message_type + ">)");
  }
  const google::protobuf::Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    reflect->MutableRepeatedMessage(m, fdesc, index)->CopyFrom(*sub_m);
  } else {
    google::protobuf::Message* copy = sub_m->New();
    copy->CopyFrom(*sub_m);
    reflect->AddAllocatedMessage(m, fdesc, copy);
  }
}

// Returns copy of the message
msg * protobuf_schema_message::get_repeated_message(
    int field_tag, int index) {
  if (m == nullptr || m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
    throw std::invalid_argument("Field is not message");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  const google::protobuf::Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    const google::protobuf::Message* original =
        &reflect->GetRepeatedMessage(*m, fdesc, index);
    google::protobuf::Message* copy = original->New();
    copy->CopyFrom(*original);
    return new protobuf_schema_message(copy);
  } else {
    throw std::out_of_range("Index out of range: " + std::to_string(index));
  }
}

void protobuf_schema_message::set_enum(int field_tag, int value) {
  if (m == nullptr || m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_ENUM) {
    throw std::invalid_argument("Field is not enum");
  }
  const google::protobuf::EnumDescriptor* edesc = fdesc->enum_type();
  if (edesc->FindValueByNumber(value) == nullptr) {
    throw std::invalid_argument("Enum doesn't have value: " +
        std::to_string(value));
  }
  m->GetReflection()->SetEnumValue(m, fdesc, value);
}

int protobuf_schema_message::get_enum(int field_tag) {
  if (m == nullptr || m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_ENUM) {
    throw std::invalid_argument("Field is not enum");
  }
  return m->GetReflection()->GetEnumValue(*m, fdesc);
}

void protobuf_schema_message::set_repeated_enum(
    int field_tag, int value, int index = -1) {
  if (m == nullptr || m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_ENUM) {
    throw std::invalid_argument("Field is not enum");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  const google::protobuf::EnumDescriptor* edesc = fdesc->enum_type();
  if (edesc->FindValueByNumber(value) == nullptr) {
    throw std::invalid_argument("Enum doesn't have value: " +
        std::to_string(value));
  }
  const google::protobuf::Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    reflect->SetRepeatedEnumValue(m, fdesc, index, value);
  } else {
    reflect->AddEnumValue(m, fdesc, value);
  }
}

int protobuf_schema_message::get_repeated_enum(int field_tag, int index) {
  if (m == nullptr || m->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      m->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc == nullptr) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_ENUM) {
    throw std::invalid_argument("Field is not enum");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  const google::protobuf::Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    return m->GetReflection()->GetRepeatedEnumValue(*m, fdesc, index);
  } else {
    throw std::out_of_range("Index out of range: " + std::to_string(index));
  }
}
