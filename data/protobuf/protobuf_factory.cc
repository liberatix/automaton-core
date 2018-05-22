#include "data/protobuf/protobuf_factory.h"
#include "data/protobuf/protobuf_schema.h"
#include "data/protobuf/protobuf_msg.h"
#include "log/log.h"

using std::string;

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

namespace automaton {
namespace core {
namespace data {
namespace protobuf {

// Protobuf schema

const std::map<schema::field_type, FieldDescriptorProto_Type>
protobuf_factory::type_to_protobuf_type {
  {schema::string, FieldDescriptorProto_Type::FieldDescriptorProto_Type_TYPE_STRING},
  {schema::int32, FieldDescriptorProto_Type::FieldDescriptorProto_Type_TYPE_INT32},
  {schema::enum_type, FieldDescriptorProto_Type::FieldDescriptorProto_Type_TYPE_ENUM},
  {schema::message_type, FieldDescriptorProto_Type::FieldDescriptorProto_Type_TYPE_MESSAGE},
};

const std::map<FieldDescriptor::Type, schema::field_type>
protobuf_factory::protobuf_type_to_type {
  {FieldDescriptor::TYPE_STRING, schema::string},
  {FieldDescriptor::TYPE_INT32, schema::int32},
  {FieldDescriptor::TYPE_ENUM, schema::enum_type},
  {FieldDescriptor::TYPE_MESSAGE, schema::message_type},
};

const std::map<FieldDescriptor::CppType, schema::field_type>
protobuf_factory::protobuf_ccptype_to_type {
  {FieldDescriptor::CPPTYPE_STRING, schema::string},
  {FieldDescriptor::CPPTYPE_INT32, schema::int32},
  {FieldDescriptor::CPPTYPE_ENUM, schema::enum_type},
  {FieldDescriptor::CPPTYPE_MESSAGE, schema::message_type},
};

protobuf_factory::protobuf_factory() {
  pool = new DescriptorPool();
  dynamic_message_factory = new DynamicMessageFactory(pool);
}

protobuf_factory::~protobuf_factory() {
  delete dynamic_message_factory;
  delete pool;
}

void protobuf_factory::register_self() {
  protobuf_factory::register_factory("protobuf", [] {
      return reinterpret_cast<factory*>(new protobuf_factory());
  });
}

void protobuf_factory::extract_nested_messages(const Descriptor* d) {
  CHECK_NOTNULL(d) << "Message descriptor is nullptr";
  int num_msg = d->nested_type_count();
  for (int i = 0; i < num_msg; i++) {
    const Descriptor* desc = d->nested_type(i);
    schemas.push_back(dynamic_message_factory->GetPrototype(desc));
    schemas_names[desc->full_name()] = schemas.size() - 1;
    extract_nested_messages(desc);
  }
}

void protobuf_factory::extract_nested_enums(const Descriptor* d) {
  CHECK_NOTNULL(d) << "Message descriptor is nullptr";
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

bool protobuf_factory::contain_invalid_data(const Descriptor* d) {
  CHECK_NOTNULL(d) << "Message descriptor is nullptr";
  if (d->oneof_decl_count() > 0) {
    return true;
  }
  int number_fields = d->field_count();
  for (int i = 0; i < number_fields; i++) {
    const FieldDescriptor* fd = d->field(i);
    if (fd->is_map() || protobuf_type_to_type.find(fd->type()) == protobuf_type_to_type.end()) {
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

void protobuf_factory::import_from_file_proto(FileDescriptorProto* fdp,
                                              const string& name,
                                              const string& package) {
  CHECK_NOTNULL(fdp) << "File descriptor proto is nullptr";
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
    throw std::runtime_error("Errors while parsing:\n" + proto_error_collector_.get_all_errors());
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

void protobuf_factory::import_schema_definition(
    schema* schema, const string& name, const string& package) {
  CHECK_NOTNULL(schema) << "schema is nullptr";
  // TODO(kari): schema need to be protobuf_factory
  import_from_file_proto(reinterpret_cast<protobuf_schema*>(schema)->get_descriptor(),
      name, package);
}

void protobuf_factory::import_schema_from_string(const string& proto_def,
                                                 const string& package,
                                                 const string& name) {
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

void protobuf_factory::dump_message_schema(int schema_id, std::ostream& ostream_) {
  CHECK_BOUNDS(schema_id, 0, schemas.size() - 1);
  CHECK_NOTNULL(schemas[schema_id]);
  CHECK_NOTNULL(schemas[schema_id]->GetDescriptor());
  const Message* m = schemas[schema_id];
  const Descriptor* desc = m->GetDescriptor();
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

std::unique_ptr<msg> protobuf_factory::new_message(int schema_id) {
  CHECK_BOUNDS(schema_id, 0, schemas.size() - 1);
  CHECK_NOTNULL(schemas[schema_id]);
  Message* m = schemas[schema_id]->New();
  return std::unique_ptr<msg>(new protobuf_msg(m));
}

std::unique_ptr<msg> protobuf_factory::new_message(const char* schema_name) {
  return new_message(get_schema_id(schema_name));
}

int protobuf_factory::get_schemas_number() const {
  return schemas.size();
}

int protobuf_factory::get_enums_number() const {
  return enums.size();
}

int protobuf_factory::get_enum_id(const string& enum_name) const {
  if (enums_names.find(enum_name) == enums_names.end()) {
    throw std::invalid_argument("No enum '" + enum_name + '\'');
  }
  return enums_names.at(enum_name);
}

void protobuf_factory::dump_enum(int enum_id, std::ostream& ostream_) {
  CHECK_BOUNDS(enum_id, 0, enums.size() - 1);
  CHECK_NOTNULL(enums[enum_id]);
  const EnumDescriptor* edesc = enums[enum_id];
  ostream_ << edesc->full_name() << " {" << std::endl;
  int values = edesc->value_count();
  for (int i = 0; i < values; i++) {
    const EnumValueDescriptor* evdesc = edesc->value(i);
    ostream_ << '\t' << evdesc->name() << " = " << evdesc->number() <<
        std::endl;
  }
  ostream_ << "}" << std::endl;
}

int protobuf_factory::get_enum_value(int enum_id, const string& value_name) const {
  CHECK_BOUNDS(enum_id, 0, enums.size() - 1);
  CHECK_NOTNULL(enums[enum_id]);
  const EnumValueDescriptor* evd = enums[enum_id]->FindValueByName(value_name);
  if (evd == nullptr) {
    throw std::invalid_argument("No enum value " + value_name);
  }
  return evd->number();
}

std::vector<std::pair<string, int> > protobuf_factory::get_enum_values(int enum_id) const {
  CHECK_BOUNDS(enum_id, 0, enums.size() - 1);
  CHECK_NOTNULL(enums[enum_id]);
  std::vector<std::pair<string, int> > result;
  const EnumDescriptor* edesc = enums[enum_id];
  int values = edesc->value_count();
  for (int i = 0; i < values; i++) {
    const EnumValueDescriptor* evdesc = edesc->value(i);
    result.push_back(std::make_pair(evdesc->name(), evdesc->number()));
  }
  return result;
}

int protobuf_factory::get_fields_number(int schema_id) const {
  CHECK_BOUNDS(schema_id, 0, schemas.size() - 1);
  CHECK_NOTNULL(schemas[schema_id]);
  CHECK_NOTNULL(schemas[schema_id]->GetDescriptor());
  const Descriptor* desc = schemas[schema_id]->GetDescriptor();
  return desc->field_count();
}

bool protobuf_factory::is_repeated(int schema_id, int field_tag) {
  CHECK_BOUNDS(schema_id, 0, schemas.size() - 1);
  CHECK_NOTNULL(schemas[schema_id]);
  CHECK_NOTNULL(schemas[schema_id]->GetDescriptor());
  const FieldDescriptor* fdesc =
      schemas[schema_id]->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc) {
    return fdesc->is_repeated();
  }
  throw std::invalid_argument("No field with tag: " + std::to_string(field_tag));
}

schema::field_info protobuf_factory::get_field_info(int schema_id, int index) const {
  CHECK_BOUNDS(schema_id, 0, schemas.size() - 1);
  CHECK_NOTNULL(schemas[schema_id]);
  CHECK_NOTNULL(schemas[schema_id]->GetDescriptor());
  const Descriptor* desc = schemas[schema_id]->GetDescriptor();
  if (index < 0 || index >= desc->field_count()) {
    throw std::out_of_range("No field with such index: " + std::to_string(index));
  }
  const FieldDescriptor* fdesc = desc->field(index);
  schema::field_type type;
  string full_type = "";
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

int protobuf_factory::get_schema_id(const string& message_name) const {
  if (schemas_names.find(message_name) == schemas_names.end()) {
    throw std::invalid_argument("No schema '" + message_name + '\'');
  }
  return schemas_names.at(message_name);
}

string protobuf_factory::get_schema_name(int schema_id) const {
  CHECK_BOUNDS(schema_id, 0, schemas.size() - 1);
  CHECK_NOTNULL(schemas[schema_id]);
  return schemas[schema_id]->GetTypeName();
}

string protobuf_factory::get_field_type(int schema_id, int tag) const {
  CHECK_BOUNDS(schema_id, 0, schemas.size() - 1);
  CHECK_NOTNULL(schemas[schema_id]);
  CHECK_NOTNULL(schemas[schema_id]->GetDescriptor());
  const FieldDescriptor* fdesc = schemas[schema_id]->GetDescriptor()->FindFieldByNumber(tag);
  if (fdesc) {
    return fdesc->cpp_type_name();
  }
  throw std::invalid_argument("No field with tag: " + std::to_string(tag));
}

string protobuf_factory::get_message_field_type(int schema_id, int field_tag) const {
  CHECK_BOUNDS(schema_id, 0, schemas.size() - 1);
  CHECK_NOTNULL(schemas[schema_id]);
  CHECK_NOTNULL(schemas[schema_id]->GetDescriptor());
  const FieldDescriptor* fdesc = schemas[schema_id]->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc) {
    if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) {
      throw std::invalid_argument("Field is not message");
    }
    return fdesc->message_type()->full_name();
  }
  throw std::invalid_argument("No field with tag: " + std::to_string(field_tag));
}

string protobuf_factory::get_enum_field_type(int schema_id, int field_tag) const {
  CHECK_BOUNDS(schema_id, 0, schemas.size() - 1);
  CHECK_NOTNULL(schemas[schema_id]);
  CHECK_NOTNULL(schemas[schema_id]->GetDescriptor());
  const FieldDescriptor* fdesc = schemas[schema_id]->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc) {
    if (fdesc->cpp_type() != FieldDescriptor::CPPTYPE_ENUM) {
      throw std::invalid_argument("Field is not enum");
    }
    return fdesc->enum_type()->full_name();
  }
  throw std::invalid_argument("No field with tag: " +
      std::to_string(field_tag));
}

int protobuf_factory::get_field_tag(int schema_id, const string& name) const {
  CHECK_BOUNDS(schema_id, 0, schemas.size() - 1);
  CHECK_NOTNULL(schemas[schema_id]);
  CHECK_NOTNULL(schemas[schema_id]->GetDescriptor());
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
      const string& message) {
  std::cerr << "*Error: line: " << line << " col: " << column << "->"
      << message.c_str() << std::endl;
  errors_number++;
  errors_list += "Error: line: " + std::to_string(line) + " col: " +
      std::to_string(column) + "->" + message.c_str() + "\n";
}

void io_error_collector::AddWarning(int line, int column,
      const string& message) {
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

string io_error_collector::get_all_errors() {
  return errors_list;
}

proto_error_collector::proto_error_collector() {
  errors_number = 0;
  errors_list = "";
}

void proto_error_collector::AddError(
    const string& schema,  // File name in which the error occurred.
    const string& element_name,  // Full name of the erroneous element.
    const Message* descriptor,  // Descriptor of the
        // erroneous element.
    ErrorLocation location,  // One of the location constants, above.
    const string& message) {  // Human-readable error message.
  std::cerr << "*Error: schema: " << schema << " <" << element_name
      << "> error message:" << message << std::endl;
  errors_number++;
  errors_list += "Error: schema: " + schema + " <" + element_name +
      "> error message:" + message + "\n";
}

void proto_error_collector::AddWarning(const string& schema,  // File
      // name in which the error occurred.
    const string& element_name,  // Full name of the erroneous element.
    const Message* descriptor,  // Descriptor of the erroneous
        // element.
    ErrorLocation location,  // One of the location constants, above.
    const string& message) {  // Human-readable error message.
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

string proto_error_collector::get_all_errors() {
  return errors_list;
}

}  // namespace protobuf
}  // namespace data
}  // namespace core
}  // namespace automaton
