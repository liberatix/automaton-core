#include "data/protobuf/protobuf_schema.h"
#include "data/protobuf/protobuf_schema_definition.h"
#include "data/protobuf/protobuf_schema_message.h"

using google::protobuf::Descriptor;
using google::protobuf::DescriptorPool;
using google::protobuf::DynamicMessageFactory;
using google::protobuf::EnumDescriptor;
using google::protobuf::EnumValueDescriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::FieldDescriptorProto_Type;
using google::protobuf::FileDescriptor;
using google::protobuf::FileDescriptorProto;
using google::protobuf::Message;

using google::protobuf::compiler::Parser;

using google::protobuf::io::IstreamInputStream;
using google::protobuf::io::Tokenizer;

// Protobuf schema

const std::map<schema::field_type, FieldDescriptorProto_Type>
protobuf_schema::type_to_protobuf_type {
  {schema::string, FieldDescriptorProto_Type::FieldDescriptorProto_Type_TYPE_STRING}, // NOLINT
  {schema::int32, FieldDescriptorProto_Type::FieldDescriptorProto_Type_TYPE_INT32}, // NOLINT
  {schema::enum_type, FieldDescriptorProto_Type::FieldDescriptorProto_Type_TYPE_ENUM}, // NOLINT
  {schema::message_type, FieldDescriptorProto_Type::FieldDescriptorProto_Type_TYPE_MESSAGE}, // NOLINT
};

const std::map<FieldDescriptor::Type, schema::field_type>
protobuf_schema::protobuf_type_to_type {
  {FieldDescriptor::TYPE_STRING, schema::string},
  {FieldDescriptor::TYPE_INT32, schema::int32},
  {FieldDescriptor::TYPE_ENUM, schema::enum_type},
  {FieldDescriptor::TYPE_MESSAGE, schema::message_type},
};

const std::map<FieldDescriptor::CppType, schema::field_type>
protobuf_schema::protobuf_ccptype_to_type {
  {FieldDescriptor::CPPTYPE_STRING, schema::string},
  {FieldDescriptor::CPPTYPE_INT32, schema::int32},
  {FieldDescriptor::CPPTYPE_ENUM, schema::enum_type},
  {FieldDescriptor::CPPTYPE_MESSAGE, schema::message_type},
};

protobuf_schema::protobuf_schema() {
  pool = new DescriptorPool();
  dynamic_message_factory = new DynamicMessageFactory(pool);
}

protobuf_schema::~protobuf_schema() {
  delete dynamic_message_factory;
  delete pool;
}

void protobuf_schema::register_self() {
  protobuf_schema::register_factory("protobuf", [] {
      return reinterpret_cast<factory*>(new protobuf_schema());
  });
}

void protobuf_schema::extract_nested_messages(const Descriptor* d) {
  if (d == nullptr) {
    throw std::invalid_argument("Message descriptor is nullptr");
  }
  int num_msg = d->nested_type_count();
  for (int i = 0; i < num_msg; i++) {
    const Descriptor* desc = d->nested_type(i);
    schemas.push_back(dynamic_message_factory->GetPrototype(desc));
    schemas_names[desc->full_name()] = schemas.size() - 1;
    extract_nested_messages(desc);
  }
}

void protobuf_schema::extract_nested_enums(
    const Descriptor* d) {
  if (d == nullptr) {
    throw std::invalid_argument("Message descriptor is nullptr");
  }
  int number_enums = d->enum_type_count();
  for (int i = 0; i < number_enums; i++) {
    const EnumDescriptor* edesc = d->enum_type(i);
    enums.push_back(edesc);
    enums_names[edesc->full_name()] = enums.size() - 1;
  }
  int num_msg = d->nested_type_count();
  for (int i = 0; i < num_msg; i++) {
    const Descriptor* desc = d->nested_type(i);
    extract_nested_enums(desc);
  }
}

bool protobuf_schema::contain_invalid_data(
  const Descriptor* d) {
  if (d == nullptr) {
    throw std::invalid_argument("Unexpected error");
  }
  if (d->oneof_decl_count() > 0) {
    return true;
  }
  int number_fields = d->field_count();
  for (int i = 0; i < number_fields; i++) {
    const FieldDescriptor* fd = d->field(i);
    if (fd->is_map() || protobuf_type_to_type.find(fd->type())
        == protobuf_type_to_type.end()) {
      return true;
    }
  }
  bool ans = false;
  int num_msg = d->nested_type_count();
  for (int i = 0; i < num_msg; i++) {
    const Descriptor* desc = d->nested_type(i);
    ans = ans | contain_invalid_data(desc);
  }
  return ans;
}

void protobuf_schema::import_from_file_proto(
    FileDescriptorProto* fdp, const std::string& name,
    const std::string& package) {
  if (fdp == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  fdp->set_package(package);
  fdp->set_name(name);

  // Checks if file with the same name already exists in the pool.
  if (pool->FindFileByName(name) != nullptr) {
    throw std::runtime_error("File with name <" + name + "> already exists.");
  }

  // Check if all dependencies are imported.
  int dependencies_number = fdp->dependency_size();
  for (int i = 0; i < dependencies_number; ++i) {
    if (pool->FindFileByName(fdp->dependency(i)) == nullptr) {
      throw std::runtime_error("Dependency <" + fdp->dependency(i) +
          "> was not found. Import it first.");
    }
  }

  const FileDescriptor* fd =
      pool->BuildFileCollectingErrors(*fdp, &proto_error_collector_);
  if (proto_error_collector_.get_number_errors() > 0) {
    throw std::runtime_error("Errors while parsing:\n" +
        proto_error_collector_.get_all_errors());
  }

  // Check for invalid data types
  int number_messages = fd->message_type_count();
  for (int i = 0; i < number_messages; i++) {
    const Descriptor* desc = fd->message_type(i);
    if (contain_invalid_data(desc)) {
      throw std::runtime_error("Message contains invalid field type!");
    }
  }

  for (int i = 0; i < number_messages; i++) {
    const Descriptor* desc = fd->message_type(i);
    schemas.push_back(dynamic_message_factory->GetPrototype(desc));
    schemas_names[desc->full_name()] = schemas.size() - 1;
    extract_nested_messages(desc);
    extract_nested_enums(desc);
  }

  int number_enums = fd->enum_type_count();
  for (int i = 0; i < number_enums; i++) {
    const EnumDescriptor* edesc = fd->enum_type(i);
    enums.push_back(edesc);
    enums_names[edesc->full_name()] = enums.size() - 1;
  }
}

void protobuf_schema::import_schema_definition(schema* schema,
    const std::string& name, const std::string& package) {
  if (schema == nullptr) {
    throw std::runtime_error("Schema reference is nullptr");
  }
  // TODO(kari): schema need to be protobuf_schema
  import_from_file_proto(
      reinterpret_cast<protobuf_schema_definition*>(schema)->get_descriptor(),
      name, package);
}

void protobuf_schema::import_schema_from_string(const std::string& proto_def,
      const std::string& package, const std::string& name) {
  FileDescriptorProto* fileproto = new FileDescriptorProto();
  std::istringstream stream(proto_def);
  IstreamInputStream is(&stream);
  Tokenizer tok(&is, &io_error_collector_);
  if (io_error_collector_.get_number_errors() > 0) {
    throw std::runtime_error(io_error_collector_.get_all_errors());
  }
  Parser parser;
  parser.RecordErrorsTo(&io_error_collector_);
  if (io_error_collector_.get_number_errors() > 0) {
    throw std::runtime_error(io_error_collector_.get_all_errors());
  }
  parser.Parse(&tok, fileproto);  // TODO(kari): Handle errors.
  if (io_error_collector_.get_number_errors() > 0) {
    throw std::runtime_error("Errors while parsing:\n" +
        io_error_collector_.get_all_errors());
  }
  import_from_file_proto(fileproto, package, name);
}

void protobuf_schema::dump_message_schema(int schema_id,
      std::ostream& ostream_) {
  if (schema_id < 0 || schema_id >= schemas.size()) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  const Message* m = schemas[schema_id];
  const Descriptor* desc = m->GetDescriptor();
  if (desc == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  ostream_ << "MessageType: " << desc->full_name() << " {" << std::endl;
  ostream_ << "Fields: " << std::endl;
  int field_count = desc->field_count();
  for (int i = 0; i < field_count; i++) {
    const FieldDescriptor* fd = desc->field(i);
    ostream_ << "\n\tName: " << fd->name() << std::endl;
    ostream_ << "\tType: ";
    if (fd->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      ostream_ << "message type: " << fd->message_type()->full_name() <<
          std::endl;
    } else if (fd->cpp_type() == FieldDescriptor::CPPTYPE_ENUM) {
      ostream_ << "enum type: " << fd->enum_type()->full_name() << std::endl;
    } else {
      ostream_ << protobuf_type_to_type.at(fd->type()) << std::endl;
    }
    ostream_ << "\tRepeated: ";
    if (fd->is_repeated()) {
      ostream_ << "YES" << std::endl;
    } else {
      ostream_ << "NO" << std::endl;
    }
    ostream_ << "\tTag: " << fd->number() << std::endl << std::endl;
  }
  ostream_ << "\n}" << std::endl;
}

msg* protobuf_schema::new_message(int schema_id) {
  if (schema_id < 0 || schema_id >= schemas.size()) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  if (schemas[schema_id] == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  Message* m = schemas[schema_id]->New();
  return new protobuf_schema_message(m);
}

msg* protobuf_schema::new_message(const char* schema_name) {
  return new_message(get_schema_id(schema_name));
}

int protobuf_schema::get_schemas_number() {
  return schemas.size();
}

int protobuf_schema::get_enums_number() {
  return enums.size();
}

int protobuf_schema::get_enum_id(const std::string& enum_name) {
  if (enums_names.find(enum_name) == enums_names.end()) {
    throw std::invalid_argument("No enum '" + enum_name + '\'');
  }
  return enums_names[enum_name];
}

void protobuf_schema::dump_enum(int enum_id, std::ostream& ostream_) {
  if (enum_id < 0 || enum_id >= enums.size()) {
    throw std::out_of_range("No enum with id: " + std::to_string(enum_id));
  }
  const EnumDescriptor* edesc = enums[enum_id];
  if (edesc == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  ostream_ << edesc->full_name() << " {" << std::endl;
  int values = edesc->value_count();
  for (int i = 0; i < values; i++) {
    const EnumValueDescriptor* evdesc = edesc->value(i);
    ostream_ << '\t' << evdesc->name() << " = " << evdesc->number() <<
        std::endl;
  }
  ostream_ << "}" << std::endl;
}

int protobuf_schema::get_enum_value(int enum_id,
    const std::string& value_name) {
  if (enum_id < 0 || enum_id >= enums.size()) {
    throw std::out_of_range("No enum with id: " + std::to_string(enum_id));
  }
  if (enums[enum_id] == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const EnumValueDescriptor* evd = enums[enum_id]->FindValueByName(value_name);
  if (evd == nullptr) {
    throw std::invalid_argument("No enum value " + value_name);
  }
  return evd->number();
}

std::vector<std::pair<std::string, int> > protobuf_schema::get_enum_values(
      int enum_id) {
  if (enum_id < 0 || enum_id >= enums.size()) {
    throw std::out_of_range("No enum with id: " + std::to_string(enum_id));
  }
  std::vector<std::pair<std::string, int> > result;
  const EnumDescriptor* edesc = enums[enum_id];
  if (edesc == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  int values = edesc->value_count();
  for (int i = 0; i < values; i++) {
    const EnumValueDescriptor* evdesc = edesc->value(i);
    result.push_back(std::make_pair(evdesc->name(), evdesc->number()));
  }
  return result;
}

int protobuf_schema::get_fields_number(int schema_id) {
  if (!(schema_id >= 0 && schema_id < schemas.size())) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  const Descriptor* desc = schemas[schema_id]->GetDescriptor();
  if (desc == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  return desc->field_count();
}

bool protobuf_schema::is_repeated(int schema_id, int field_tag) {
  if (!(schema_id >= 0 && schema_id < schemas.size())) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  if (schemas[schema_id]->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const FieldDescriptor* fdesc =
      schemas[schema_id]->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc) {
    return fdesc->is_repeated();
  }
  throw std::invalid_argument("No field with tag: " +
      std::to_string(field_tag));
}

schema::field_info protobuf_schema::get_field_info(int schema_id,
      int index) {
  if (!(schema_id >= 0 && schema_id < schemas.size())) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  const Descriptor* desc = schemas[schema_id]->GetDescriptor();
  if (desc == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  if (index < 0 || index >= desc->field_count()) {
    throw std::out_of_range("No field with such index: " +
        std::to_string(index));
  }
  const FieldDescriptor* fdesc = desc->field(index);
  schema::field_type type;
  std::string full_type = "";
  if (fdesc->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
    full_type = fdesc->message_type()->full_name();
    type = schema::message_type;
  } else if (fdesc->cpp_type() == FieldDescriptor::CPPTYPE_ENUM) {
    full_type = fdesc->enum_type()->full_name();
    type = schema::enum_type;
  } else {
    type = protobuf_ccptype_to_type.at(fdesc->cpp_type());
  }
  return schema::field_info(
      fdesc->number(),
      type,
      fdesc->name(),
      full_type,
      fdesc->is_repeated());
}

int protobuf_schema::get_schema_id(const std::string& message_name) {
  if (schemas_names.find(message_name) == schemas_names.end()) {
    throw std::invalid_argument("No schema '" + message_name + '\'');
  }
  return schemas_names[message_name];
}

std::string protobuf_schema::get_schema_name(int schema_id) {
  if (schema_id < 0 || schema_id >= schemas.size()) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  if (schemas[schema_id] == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  return schemas[schema_id]->GetTypeName();
}

std::string protobuf_schema::get_field_type(int schema_id, int tag) {
  if (schema_id < 0 || schema_id >= schemas.size()) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  if (schemas[schema_id]->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const FieldDescriptor* fdesc =
      schemas[schema_id]->GetDescriptor()->FindFieldByNumber(tag);
  if (fdesc) {
    return fdesc->cpp_type_name();
  }
  throw std::invalid_argument("No field with tag: " + std::to_string(tag));
}

std::string protobuf_schema::get_message_field_type(int schema_id,
      int field_tag) {
  if (schema_id < 0 || schema_id >= schemas.size()) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  if (schemas[schema_id]->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const FieldDescriptor* fdesc =
      schemas[schema_id]->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc) {
    if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) {
      throw std::invalid_argument("Field is not message");
    }
    return fdesc->message_type()->full_name();
  }
  throw std::invalid_argument("No field with tag: " +
      std::to_string(field_tag));
}

std::string protobuf_schema::get_enum_field_type(int schema_id, int field_tag) {
  if (schema_id < 0 || schema_id >= schemas.size()) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  if (schemas[schema_id]->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const FieldDescriptor* fdesc =
      schemas[schema_id]->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc) {
    if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_ENUM) {
      throw std::invalid_argument("Field is not enum");
    }
    return fdesc->enum_type()->full_name();
  }
  throw std::invalid_argument("No field with tag: " +
      std::to_string(field_tag));
}

int protobuf_schema::get_field_tag(int schema_id, const std::string& name) {
  if (schema_id < 0 || schema_id >= schemas.size()) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  if (schemas[schema_id]->GetDescriptor() == nullptr) {
    throw std::runtime_error("Unexpected error");
  }
  const FieldDescriptor* fdesc =
      schemas[schema_id]->GetDescriptor()->FindFieldByName(name);
  if (fdesc) {
    return fdesc->number();
  }
  throw std::invalid_argument("No field with name: " + name);
}

// Error collectors helper classes

io_error_collector::io_error_collector() {
  errors_number = 0;
  errors_list = "";
}

void io_error_collector::AddError(int line, int column,
      const std::string& message) {
  std::cerr << "*Error: line: " << line << " col: " << column << "->"
      << message.c_str() << std::endl;
  errors_number++;
  errors_list += "Error: line: " + std::to_string(line) + " col: " +
      std::to_string(column) + "->" + message.c_str() + "\n";
}

void io_error_collector::AddWarning(int line, int column,
      const std::string& message) {
  std::cerr << "*Warning: line: " << line << " col: " << column << "->"
      << message.c_str() << std::endl;
  errors_list += "Warning: line: " + std::to_string(line) + " col: " +
      std::to_string(column) + "->" + message.c_str() + "\n";
}

void io_error_collector::clear_errors() {
  errors_number = 0;
  errors_list = "";
}

int io_error_collector::get_number_errors() {
  return errors_number;
}

std::string io_error_collector::get_all_errors() {
  return errors_list;
}

proto_error_collector::proto_error_collector() {
  errors_number = 0;
  errors_list = "";
}

void proto_error_collector::AddError(
    const std::string& schema,  // File name in which the error occurred.
    const std::string& element_name,  // Full name of the erroneous element.
    const Message* descriptor,  // Descriptor of the
        // erroneous element.
    ErrorLocation location,  // One of the location constants, above.
    const std::string& message) {  // Human-readable error message.
  std::cerr << "*Error: schema: " << schema << " <" << element_name
      << "> error message:" << message << std::endl;
  errors_number++;
  errors_list += "Error: schema: " + schema + " <" + element_name +
      "> error message:" + message + "\n";
}

void proto_error_collector::AddWarning(const std::string& schema,  // File
      // name in which the error occurred.
    const std::string& element_name,  // Full name of the erroneous element.
    const Message* descriptor,  // Descriptor of the erroneous
        // element.
    ErrorLocation location,  // One of the location constants, above.
    const std::string& message) {  // Human-readable error message.
  std::cerr << "*Warning: schema: " << schema << " <" <<
      element_name << "> error message:" << message << std::endl;
  errors_number++;
  errors_list += "Warning: schema: " + schema + " <" + element_name
      + "> error message:" + message + "\n";
}

void proto_error_collector::clear_errors() {
  errors_number = 0;
  errors_list = "";
}

int proto_error_collector::get_number_errors() {
  return errors_number;
}

std::string proto_error_collector::get_all_errors() {
  return errors_list;
}
