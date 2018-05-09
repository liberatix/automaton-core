#include "data/protobuf_schema.h"
#include "data/protobuf_schema_definition.h"

using google::protobuf::Arena;
using google::protobuf::DescriptorProto;
using google::protobuf::EnumDescriptorProto;
using google::protobuf::EnumValueDescriptorProto;
using google::protobuf::FieldDescriptorProto;
using google::protobuf::FieldDescriptorProto_Label;
using google::protobuf::FieldDescriptorProto_Type;
using google::protobuf::FileDescriptorProto;

// Protobuf schema definition

protobuf_schema_definition::protobuf_schema_definition() {
  file_descriptor_proto = Arena::Create<
      FileDescriptorProto>(&arena);
  file_descriptor_proto->set_syntax("proto3");
}

protobuf_schema_definition::~protobuf_schema_definition() {}

FileDescriptorProto*
    protobuf_schema_definition::get_descriptor() {
        return file_descriptor_proto;
}

void protobuf_schema_definition::register_self() {
  schema::register_factory("protobuf", [] {
      return reinterpret_cast<schema*>
          (new protobuf_schema_definition());
  });
}

void protobuf_schema_definition::add_dependency(const std::string&
    schema_name) {
  file_descriptor_proto->add_dependency(schema_name);
}

int protobuf_schema_definition::create_message(const std::string&
    message_name) {
  messages.push_back(
      Arena::Create
          <DescriptorProto>(&arena));
  messages[messages.size() - 1]->set_name(message_name);
  return messages.size() - 1;
}

int protobuf_schema_definition::create_enum(const std::string& enum_name) {
  enums.push_back(Arena::Create<
      EnumDescriptorProto>(&arena));
  enums[enums.size() - 1]->set_name(enum_name);
  return enums.size() - 1;
}

void protobuf_schema_definition::add_enum_value(int enum_id, const std::string&
    value_name, int value) {
  if (enum_id < 0 || enum_id >= enums.size()) {
    throw std::out_of_range("No enum with id: " + std::to_string(enum_id));
  }
  EnumDescriptorProto* edp = enums[enum_id];
  if (edp == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  EnumValueDescriptorProto* field = edp->add_value();
  field->set_name(value_name);
  field->set_number(value);
}

void protobuf_schema_definition::add_nested_message(int message_id,
    int sub_message_id) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  if (sub_message_id < 0 || sub_message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  if (messages[message_id] == nullptr || messages[sub_message_id] == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  DescriptorProto* dpr = messages[message_id];
  DescriptorProto* m = dpr->add_nested_type();
  m->CopyFrom(*messages[sub_message_id]);
}

void protobuf_schema_definition::add_message(int message_id) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  if (messages[message_id] == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  DescriptorProto* m =
      file_descriptor_proto->add_message_type();
  m->CopyFrom(*messages[message_id]);
}

void protobuf_schema_definition::add_enum(int enum_id, int message_id
    = -1) {
  if (enum_id < 0 || enum_id >= enums.size()) {
    throw std::out_of_range("No enum with id: " + std::to_string(enum_id));
  }
  if (enums[enum_id] == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  if (message_id == -1) {
    EnumDescriptorProto* edp =
        file_descriptor_proto->add_enum_type();
    edp->CopyFrom(*enums[enum_id]);
  } else {
    if (message_id < 0 || message_id >= messages.size()) {
      throw std::out_of_range("No message with id: "
          + std::to_string(message_id));
    }
    if (messages[message_id] == nullptr) {
      throw std::runtime_error("Unexpected error");
    }
    DescriptorProto* dpr = messages[message_id];
    EnumDescriptorProto* edp = dpr->add_enum_type();
    edp->CopyFrom(*enums[enum_id]);
  }
}

void protobuf_schema_definition::add_scalar_field(
    schema::field_info field, int message_id) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  if (field.type == schema::message_type ||
      field.type == schema::enum_type ||
      field.type == schema::unknown) {
    throw std::invalid_argument("Wrong field type");
  }
  DescriptorProto* dpr = messages[message_id];
  if (dpr == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  FieldDescriptorProto* field_proto = dpr->add_field();
  field_proto->set_name(field.name);
  field_proto->set_type(protobuf_schema::type_to_protobuf_type.at(field.type));
  field_proto->set_number(field.tag);
  if (field.is_repeated) {
    field_proto->set_label(
        FieldDescriptorProto_Label::FieldDescriptorProto_Label_LABEL_REPEATED);
  }
}

void protobuf_schema_definition::add_enum_field(
    schema::field_info field, int message_id) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  if (field.type != schema::enum_type) {
    throw std::invalid_argument("Wrong field type");
  }
  DescriptorProto* dpr = messages[message_id];
  if (dpr == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  FieldDescriptorProto* field_proto = dpr->add_field();
  field_proto->set_name(field.name);
  field_proto->set_type(
      FieldDescriptorProto_Type::FieldDescriptorProto_Type_TYPE_ENUM);
  field_proto->set_number(field.tag);
  field_proto->set_type_name(field.fully_qualified_type);
  if (field.is_repeated) {
    field_proto->set_label(
        FieldDescriptorProto_Label::FieldDescriptorProto_Label_LABEL_REPEATED);
  }
}

void protobuf_schema_definition::add_message_field(
    schema::field_info field, int message_id) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  if (field.type != schema::message_type) {
    throw std::invalid_argument("Wrong field type");
  }
  DescriptorProto* dpr = messages[message_id];
  if (dpr == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  FieldDescriptorProto* field_proto = dpr->add_field();
  field_proto->set_name(field.name);
  field_proto->set_type(
      FieldDescriptorProto_Type::FieldDescriptorProto_Type_TYPE_MESSAGE);
  field_proto->set_number(field.tag);
  field_proto->set_type_name(field.fully_qualified_type);
  if (field.is_repeated) {
    field_proto->set_label(
        FieldDescriptorProto_Label::FieldDescriptorProto_Label_LABEL_REPEATED);
  }
}
