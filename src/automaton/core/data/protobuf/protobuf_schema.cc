#include "automaton/core/data/protobuf/protobuf_factory.h"

#include <google/protobuf/compiler/parser.h>

#include "automaton/core/data/protobuf/protobuf_schema.h"
#include "automaton/core/log/log.h"

using google::protobuf::DescriptorProto;
using google::protobuf::EnumDescriptorProto;
using google::protobuf::EnumValueDescriptorProto;
using google::protobuf::FieldDescriptorProto;
using google::protobuf::FieldDescriptorProto_Label;
using google::protobuf::FieldDescriptorProto_Type;
using google::protobuf::FileDescriptorProto;

using google::protobuf::compiler::Parser;

using google::protobuf::io::IstreamInputStream;
using google::protobuf::io::Tokenizer;

namespace automaton {
namespace core {
namespace data {
namespace protobuf {

/**
This is helper class which is used while parsing proto file.
**/
class io_error_collector : public google::protobuf::io::ErrorCollector {
 public:
  io_error_collector() {
  errors_number = 0;
  errors_list = "";
  }

  void AddError(int line, int column, const std::string& message) {
    std::cerr << "*Error: line: " << line << " col: " << column << "->"
        << message.c_str() << std::endl;
    errors_number++;
    errors_list += "Error: line: " + std::to_string(line) + " col: " +
        std::to_string(column) + "->" + message.c_str() + "\n";
  }

  void AddWarning(int line, int column, const std::string& message) {
    std::cerr << "*Warning: line: " << line << " col: " << column << "->"
        << message.c_str() << std::endl;
    errors_list += "Warning: line: " + std::to_string(line) + " col: " +
        std::to_string(column) + "->" + message.c_str() + "\n";
  }

  void clear_errors() {
    errors_number = 0;
    errors_list = "";
  }

  int get_number_errors() {
    return errors_number;
  }

  std::string get_all_errors() {
    return errors_list;
  }

 private:
  int errors_number;
  std::string errors_list;
};


// Protobuf schema definition

protobuf_schema::protobuf_schema() {
  file_descriptor_proto.reset(new FileDescriptorProto());
  file_descriptor_proto->set_syntax("proto3");
}

protobuf_schema::protobuf_schema(const std::string& proto_def) {
  io_error_collector io_error_collector_;

  file_descriptor_proto.reset(new FileDescriptorProto());
  std::istringstream stream(proto_def);
  IstreamInputStream is(&stream);
  VLOG(9) << "Tokenizing .proto file";
  Tokenizer tok(&is, &io_error_collector_);
  if (io_error_collector_.get_number_errors() > 0) {
    throw std::runtime_error(io_error_collector_.get_all_errors());
  }
  Parser parser;
  parser.RecordErrorsTo(&io_error_collector_);
  if (io_error_collector_.get_number_errors() > 0) {
    throw std::runtime_error(io_error_collector_.get_all_errors());
  }
  VLOG(9) << "Parsing tokenized proto";
  parser.Parse(&tok, file_descriptor_proto.get());  // TODO(kari): Handle errors.
  if (io_error_collector_.get_number_errors() > 0) {
    throw std::runtime_error("Errors while parsing:\n" +
        io_error_collector_.get_all_errors());
  }
  VLOG(9) << "Parsing complete";
}

protobuf_schema::~protobuf_schema() {}

FileDescriptorProto* protobuf_schema::get_file_descriptor_proto() {
  return file_descriptor_proto.get();
}

void protobuf_schema::register_self() {
  schema::register_factory("protobuf",
      [] {
        return reinterpret_cast<schema*>(new protobuf_schema());
      });
}

void protobuf_schema::add_dependency(const std::string& schema_name) {
  LOG(INFO) << "Adding dependency on " << schema_name << " to " << file_descriptor_proto->name();
  file_descriptor_proto->add_dependency(schema_name);
}

int protobuf_schema::create_message(const std::string& message_name) {
  messages.push_back(new DescriptorProto());
  messages[messages.size() - 1]->set_name(message_name);
  return messages.size() - 1;
}

int protobuf_schema::create_enum(const std::string& enum_name) {
  enums.push_back(new EnumDescriptorProto());
  enums[enums.size() - 1]->set_name(enum_name);
  return enums.size() - 1;
}

void protobuf_schema::add_enum_value(int enum_id, const std::string& value_name, int value) {
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

void protobuf_schema::add_nested_message(int message_id, int sub_message_id) {
  CHECK_BOUNDS(message_id, 0, messages.size() - 1) << "message_id out of bounds";
  CHECK_BOUNDS(sub_message_id, 0, messages.size() - 1) << "sub_message_id out of bounds";
  if (messages[message_id] == nullptr || messages[sub_message_id] == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  DescriptorProto* dpr = messages[message_id];
  DescriptorProto* m = dpr->add_nested_type();
  m->CopyFrom(*messages[sub_message_id]);
}

void protobuf_schema::add_message(int message_id) {
  CHECK_BOUNDS(message_id, 0, messages.size() - 1) << "message_id out of bounds";
  if (messages[message_id] == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  DescriptorProto* m = file_descriptor_proto->add_message_type();
  m->CopyFrom(*messages[message_id]);
}

void protobuf_schema::add_enum(int enum_id, int message_id = -1) {
  if (enum_id < 0 || enum_id >= enums.size()) {
    throw std::out_of_range("No enum with id: " + std::to_string(enum_id));
  }
  if (enums[enum_id] == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  if (message_id == -1) {
    EnumDescriptorProto* edp = file_descriptor_proto->add_enum_type();
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

void protobuf_schema::add_scalar_field(schema::field_info field, int message_id) {
  CHECK_BOUNDS(message_id, 0, messages.size() - 1) << "message_id out of bounds";
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
  field_proto->set_type(protobuf_factory::type_to_protobuf_type.at(field.type));
  field_proto->set_number(field.tag);
  if (field.is_repeated) {
    field_proto->set_label(FieldDescriptorProto_Label::FieldDescriptorProto_Label_LABEL_REPEATED);
  }
}

void protobuf_schema::add_enum_field(schema::field_info field, int message_id) {
  CHECK_BOUNDS(message_id, 0, messages.size() - 1) << "message_id out of bounds";
  if (field.type != schema::enum_type) {
    throw std::invalid_argument("Wrong field type");
  }
  DescriptorProto* dpr = messages[message_id];
  if (dpr == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  FieldDescriptorProto* field_proto = dpr->add_field();
  field_proto->set_name(field.name);
  field_proto->set_type(FieldDescriptorProto_Type::FieldDescriptorProto_Type_TYPE_ENUM);
  field_proto->set_number(field.tag);
  field_proto->set_type_name(field.fully_qualified_type);
  if (field.is_repeated) {
    field_proto->set_label(FieldDescriptorProto_Label::FieldDescriptorProto_Label_LABEL_REPEATED);
  }
}

void protobuf_schema::add_message_field(schema::field_info field, int message_id) {
  CHECK_BOUNDS(message_id, 0, messages.size() - 1) << "message_id out of bounds";
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

/**
  Serializes schema to JSON string.
*/
bool protobuf_schema::to_json(std::string* output) {
  // TODO(asen): To be implemented.
  return false;
}

/**
  Deserializes schema from JSON string.
*/
bool protobuf_schema::from_json(const std::string& input) {
  // TODO(asen): To be implemented.
  return false;
}

}  // namespace protobuf
}  // namespace data
}  // namespace core
}  // namespace automaton
