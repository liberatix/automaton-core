#include "schema/protobuf_schema.h"
#include "schema/protobuf_schema_definition.h"

// Protobuf schema

const std::map<schema_definition::field_type,
               google::protobuf::FieldDescriptorProto_Type>
protobuf_schema::type_to_protobuf_type {
  {schema_definition::string,
      google::protobuf::FieldDescriptorProto_Type_TYPE_STRING},
  {schema_definition::int32,
      google::protobuf::FieldDescriptorProto_Type_TYPE_INT32},
  {schema_definition::enum_type,
      google::protobuf::FieldDescriptorProto_Type_TYPE_ENUM},
  {schema_definition::message_type,
      google::protobuf::FieldDescriptorProto_Type_TYPE_MESSAGE}
};

const std::map<google::protobuf::FieldDescriptor::Type,
               schema_definition::field_type>
protobuf_schema::protobuf_type_to_type {
  {google::protobuf::FieldDescriptor::Type::TYPE_STRING,
      schema_definition::string},
  {google::protobuf::FieldDescriptor::Type::TYPE_INT32,
      schema_definition::int32},
  {google::protobuf::FieldDescriptor::Type::TYPE_ENUM,
      schema_definition::enum_type},
  {google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE,
      schema_definition::message_type}
};

const std::map<google::protobuf::FieldDescriptor::CppType,
               schema_definition::field_type>
protobuf_schema::protobuf_ccptype_to_type {
  {google::protobuf::FieldDescriptor::CPPTYPE_STRING,
      schema_definition::string},
  {google::protobuf::FieldDescriptor::CPPTYPE_INT32,
      schema_definition::int32},
  {google::protobuf::FieldDescriptor::CPPTYPE_ENUM,
      schema_definition::enum_type},
  {google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE,
      schema_definition::message_type}
};

protobuf_schema::protobuf_schema() {
  pool = google::protobuf::Arena::Create
      <google::protobuf::DescriptorPool>(&arena);
  dynamic_message_factory = google::protobuf::Arena::Create
      <google::protobuf::DynamicMessageFactory>(&arena, pool);
}

protobuf_schema::~protobuf_schema() {}

void protobuf_schema::register_self() {
  protobuf_schema::register_factory("protobuf", [] {
      return reinterpret_cast<schema*>(new protobuf_schema());
  });
}

int protobuf_schema::insert_message(google::protobuf::Message* message) {
  if (!free_message_spots.empty()) {
    int spot = free_message_spots.top();
    free_message_spots.pop();
    messages[spot] = message;
    return spot;
  }
  messages.push_back(message);
  return messages.size() - 1;
}

void protobuf_schema::extract_nested_messages(
    const google::protobuf::Descriptor* d) {
  if (d == NULL) {
    throw std::invalid_argument("Message descriptor is NULL");
  }
  int num_msg = d->nested_type_count();
  for (int i = 0; i < num_msg; i++) {
    const google::protobuf::Descriptor* desc = d->nested_type(i);
    schemas.push_back(dynamic_message_factory->GetPrototype(desc));
    schemas_names[desc->full_name()] = schemas.size() - 1;
    extract_nested_messages(desc);
  }
}
void protobuf_schema::extract_nested_enums(
    const google::protobuf::Descriptor* d) {
  if (d == NULL) {
    throw std::invalid_argument("Message descriptor is NULL");
  }
  int number_enums = d->enum_type_count();
  for (int i = 0; i < number_enums; i++) {
    const google::protobuf::EnumDescriptor* edesc = d->enum_type(i);
    enums.push_back(edesc);
    enums_names[edesc->full_name()] = enums.size() - 1;
  }
  int num_msg = d->nested_type_count();
  for (int i = 0; i < num_msg; i++) {
    const google::protobuf::Descriptor* desc = d->nested_type(i);
    extract_nested_enums(desc);
  }
}

bool protobuf_schema::contain_invalid_data(const google::protobuf::Descriptor*
    d) {
  if (d == NULL) {
    throw std::invalid_argument("Unexpected error");
  }
  if (d->oneof_decl_count() > 0) {
    return true;
  }
  int number_fields = d->field_count();
  for (int i = 0; i < number_fields; i++) {
    const google::protobuf::FieldDescriptor* fd = d->field(i);
    if (fd->is_map() || protobuf_type_to_type.find(fd->type())
        == protobuf_type_to_type.end()) {
      return true;
    }
  }
  bool ans = false;
  int num_msg = d->nested_type_count();
  for (int i = 0; i < num_msg; i++) {
    const google::protobuf::Descriptor* desc = d->nested_type(i);
    ans = ans | contain_invalid_data(desc);
  }
  return ans;
}

void protobuf_schema::import_from_file_proto(
    google::protobuf::FileDescriptorProto* fdp, const std::string& name,
    const std::string& package) {
  if (fdp == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  fdp->set_package(package);
  fdp->set_name(name);

  // Checks if file with the same name already exists in the pool.
  if (pool->FindFileByName(name) != NULL) {
    throw std::runtime_error("File with name <" + name + "> already exists.");
  }

  // Check if all dependencies are imported.
  int dependencies_number = fdp->dependency_size();
  for (int i = 0; i < dependencies_number; ++i) {
    if (pool->FindFileByName(fdp->dependency(i)) == NULL) {
      throw std::runtime_error("Dependency <" + fdp->dependency(i) +
          "> was not found. Import it first.");
    }
  }

  const google::protobuf::FileDescriptor* fd =
      pool->BuildFileCollectingErrors(*fdp, &proto_error_collector_);
  if (proto_error_collector_.get_number_errors() > 0) {
    throw std::runtime_error("Errors while parsing:\n" +
        proto_error_collector_.get_all_errors());
  }

  // Check for invalid data types
  int number_messages = fd->message_type_count();
  for (int i = 0; i < number_messages; i++) {
    const google::protobuf::Descriptor* desc = fd->message_type(i);
    if (contain_invalid_data(desc)) {
      throw std::runtime_error("Message contains invalid field type!");
    }
  }

  for (int i = 0; i < number_messages; i++) {
    const google::protobuf::Descriptor* desc = fd->message_type(i);
    schemas.push_back(dynamic_message_factory->GetPrototype(desc));
    schemas_names[desc->full_name()] = schemas.size() - 1;
    extract_nested_messages(desc);
    extract_nested_enums(desc);
  }

  int number_enums = fd->enum_type_count();
  for (int i = 0; i < number_enums; i++) {
    const google::protobuf::EnumDescriptor* edesc = fd->enum_type(i);
    enums.push_back(edesc);
    enums_names[edesc->full_name()] = enums.size() - 1;
  }
}

void protobuf_schema::import_schema_definition(schema_definition* schema,
    const std::string& name, const std::string& package) {
  if (schema == NULL) {
    throw std::runtime_error("Schema reference is NULL");
  }
  // TODO(kari): schema need to be protobuf_schema
  import_from_file_proto(
      reinterpret_cast<protobuf_schema_definition*>(schema)->get_descriptor(),
      name, package);
}

void protobuf_schema::import_schema_from_string(const std::string& proto_def,
      const std::string& package, const std::string& name) {
  google::protobuf::FileDescriptorProto* fileproto =
      google::protobuf::Arena::Create<
      google::protobuf::FileDescriptorProto>(&arena);
  std::istringstream stream(proto_def);
  google::protobuf::io::IstreamInputStream is(&stream);
  google::protobuf::io::Tokenizer tok(&is, &io_error_collector_);
  if (io_error_collector_.get_number_errors() > 0) {
    throw std::runtime_error(io_error_collector_.get_all_errors());
  }
  google::protobuf::compiler::Parser parser;
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
  const google::protobuf::Message* m = schemas[schema_id];
  const google::protobuf::Descriptor* desc = m->GetDescriptor();
  if (desc == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  ostream_ << "MessageType: " << desc->full_name() << " {" << std::endl;
  ostream_ << "Fields: " << std::endl;
  int field_count = desc->field_count();
  for (int i = 0; i < field_count; i++) {
    const google::protobuf::FieldDescriptor* fd = desc->field(i);
    ostream_ << "\n\tName: " << fd->name() << std::endl;
    ostream_ << "\tType: ";
    if (fd->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
      ostream_ << "message type: " << fd->message_type()->full_name() <<
          std::endl;
    } else if (fd->cpp_type() ==
        google::protobuf::FieldDescriptor::CPPTYPE_ENUM) {
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

int protobuf_schema::new_message(int schema_id) {
  if (schema_id < 0 || schema_id >= schemas.size()) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  if (schemas[schema_id] == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  google::protobuf::Message* m = schemas[schema_id]-> New();
  return insert_message(m);
}

int protobuf_schema::create_copy_message(int message_id) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  if (messages[message_id] == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  google::protobuf::Message* copy = messages[message_id]-> New();
  copy->CopyFrom(*messages[message_id]);
  return insert_message(copy);
}

void protobuf_schema::delete_message(int message_id) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  delete messages[message_id];
  messages[message_id] = NULL;
  free_message_spots.push(message_id);
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
  const google::protobuf::EnumDescriptor* edesc = enums[enum_id];
  if (edesc == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  ostream_ << edesc->full_name() << " {" << std::endl;
  int values = edesc->value_count();
  for (int i = 0; i < values; i++) {
    const google::protobuf::EnumValueDescriptor* evdesc = edesc->value(i);
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
  if (enums[enum_id] == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::EnumValueDescriptor* evd =
      enums[enum_id]->FindValueByName(value_name);
  if (evd == NULL) {
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
  const google::protobuf::EnumDescriptor* edesc = enums[enum_id];
  if (edesc == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  int values = edesc->value_count();
  for (int i = 0; i < values; i++) {
    const google::protobuf::EnumValueDescriptor* evdesc = edesc->value(i);
    result.push_back(std::make_pair(evdesc->name(), evdesc->number()));
  }
  return result;
}

int protobuf_schema::get_fields_number(int schema_id) {
  if (!(schema_id >= 0 && schema_id < schemas.size())) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  const google::protobuf::Descriptor* desc =
      schemas[schema_id]->GetDescriptor();
  if (desc == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  return desc->field_count();
}

bool protobuf_schema::is_repeated(int schema_id, int field_tag) {
  if (!(schema_id >= 0 && schema_id < schemas.size())) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  if (schemas[schema_id]->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      schemas[schema_id]->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc) {
    return fdesc->is_repeated();
  }
  throw std::invalid_argument("No field with tag: " + field_tag);
}

schema_definition::field_info protobuf_schema::get_field_info(int schema_id,
      int index) {
  if (!(schema_id >= 0 && schema_id < schemas.size())) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  const google::protobuf::Descriptor* desc =
      schemas[schema_id]->GetDescriptor();
  if (desc == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  if (index < 0 || index >= desc->field_count()) {
    throw std::out_of_range("No field with such index: " +
        std::to_string(index));
  }
  const google::protobuf::FieldDescriptor* fdesc = desc->field(index);
  schema_definition::field_type type;
  std::string full_type = "";
  if (fdesc->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
    full_type = fdesc->message_type()->full_name();
    type = schema_definition::field_type::message_type;
  } else if (fdesc->cpp_type() ==
      google::protobuf::FieldDescriptor::CPPTYPE_ENUM) {
    full_type = fdesc->enum_type()->full_name();
    type = schema_definition::field_type::enum_type;
  } else {
    type = protobuf_ccptype_to_type.at(fdesc->cpp_type());
  }
  return schema_definition::field_info(
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
  if (schemas[schema_id] == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  return schemas[schema_id]->GetTypeName();
}
/*
std::string protobuf_schema::get_message_type(int message_id) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: " +
        std::to_string(message_id));
  }
  return messages[message_id]->GetTypeName();
}
*/
int protobuf_schema::get_message_schema_id(int message_id) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: " +
        std::to_string(message_id));
  }
  if (messages[message_id] == NULL) {
    throw std::runtime_error("Unexpected error: No message");
  }
  return get_schema_id(messages[message_id]->GetTypeName());
}

std::string protobuf_schema::get_field_type(int schema_id, int tag) {
  if (schema_id < 0 || schema_id >= schemas.size()) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  if (schemas[schema_id]->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
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
  if (schemas[schema_id]->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      schemas[schema_id]->GetDescriptor()
      -> FindFieldByNumber(field_tag);
  if (fdesc) {
    if (fdesc->cpp_type() !=
        google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
      throw std::invalid_argument("Field is not message");
    }
    return fdesc->message_type()->full_name();
  }
  throw std::invalid_argument("No field with tag: " + field_tag);
}

std::string protobuf_schema::get_enum_field_type(int schema_id, int field_tag) {
  if (schema_id < 0 || schema_id >= schemas.size()) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  if (schemas[schema_id]->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      schemas[schema_id]->GetDescriptor()->FindFieldByNumber(field_tag);
  if (fdesc) {
    if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_ENUM) {
      throw std::invalid_argument("Field is not enum");
    }
    return fdesc->enum_type()->full_name();
  }
  throw std::invalid_argument("No field with tag: " + field_tag);
}

int protobuf_schema::get_field_tag(int schema_id, const std::string& name) {
  if (schema_id < 0 || schema_id >= schemas.size()) {
    throw std::out_of_range("No schema with id: " + std::to_string(schema_id));
  }
  if (schemas[schema_id]->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc =
      schemas[schema_id]->GetDescriptor()->FindFieldByName(name);
  if (fdesc) {
    return fdesc->number();
  }
  throw std::invalid_argument("No field with name: " + name);
}

int protobuf_schema::get_repeated_field_size(int message_id, int field_tag) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
      -> FindFieldByNumber(field_tag);
  if (fdesc == NULL) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  return m->GetReflection()->FieldSize(*m, fdesc);
}

bool protobuf_schema::serialize_message(int message_id, std::string* output) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (output == NULL) {
    throw std::invalid_argument("No output provided");
  }
  if (m == NULL) {
    throw std::runtime_error("Unexpected error: No message");
  }
  return m->SerializeToString(output);  // TODO(kari): Handle errors.
}

bool protobuf_schema::deserialize_message(int message_id,
      const std::string& input) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL) {
    throw std::runtime_error("Unexpected error: No message");
  }
  return m->ParseFromString(input);  // TODO(kari): Handle errors.
}

std::string protobuf_schema::to_string(int message_id) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  if (messages[message_id] == NULL) {
    throw std::runtime_error("Unexpected error: No message");
  }
  return messages[message_id]->DebugString();
}

void protobuf_schema::set_string(int message_id, int field_tag,
      const std::string& value) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error: No message");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
      -> FindFieldByNumber(field_tag);

  if (fdesc == NULL) {
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

std::string protobuf_schema::get_string(int message_id, int field_tag) {
  if (message_id < 0 || message_id >= messages.size()) {
      throw std::out_of_range("No message with id: " +
          std::to_string(message_id));
  }

  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
      -> FindFieldByNumber(field_tag);
  if (fdesc == NULL) {
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

void protobuf_schema::set_repeated_string(int message_id, int field_tag,
      const std::string& value, int index = -1) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
      -> FindFieldByNumber(field_tag);
  if (fdesc == NULL) {
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
  if (index >= 0 && index < reflect->FieldSize(*messages[message_id], fdesc)) {
    reflect->SetRepeatedString(m, fdesc, index, value);
  } else {
    reflect->AddString(m, fdesc, value);
  }
}

std::string protobuf_schema::get_repeated_string(int message_id, int field_tag,
      int index) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
      -> FindFieldByNumber(field_tag);
  if (fdesc == NULL) {
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

void protobuf_schema::set_int32(int message_id, int field_tag, int32_t value) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
     ->FindFieldByNumber(field_tag);
  if (fdesc == NULL) {
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

int32_t protobuf_schema::get_int32(int message_id, int field_tag) {
  if (message_id < 0 || message_id >= messages.size()) {
      throw std::out_of_range("No message with id: " +
          std::to_string(message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
     ->FindFieldByNumber(field_tag);
  if (fdesc == NULL) {
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

void protobuf_schema::set_repeated_int32(int message_id, int field_tag,
    int32_t value, int index = -1) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
     ->FindFieldByNumber(field_tag);
  if (fdesc == NULL) {
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
  if (index >= 0 && index < reflect->FieldSize(*messages[message_id], fdesc)) {
    reflect->SetRepeatedInt32(m, fdesc, index, value);
  } else {
    reflect->AddInt32(m, fdesc, value);
  }
}

int32_t protobuf_schema::get_repeated_int32(int message_id, int field_tag,
      int index) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
      -> FindFieldByNumber(field_tag);
  if (fdesc == NULL) {
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

void protobuf_schema::set_message(int message_id, int field_tag, int
      sub_message_id) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  if (!(sub_message_id >= 0 && sub_message_id < messages.size())) {
    throw std::out_of_range("No sub message with id: " +
        std::to_string(sub_message_id));
  }  // throw
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error: No message");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
      -> FindFieldByNumber(field_tag);
  if (fdesc == NULL) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->is_repeated()) {
    throw std::invalid_argument("Field is repeated");
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
    throw std::invalid_argument("Field is not message");
  }
  if (messages[sub_message_id] == NULL || messages[sub_message_id]->
        GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error: No message");
  }
  std::string message_type = fdesc->message_type()->full_name();  // Error
  std::string sub_message_type = messages[sub_message_id]->
      GetDescriptor()->full_name();
  if (message_type.compare(sub_message_type)) {
    throw std::invalid_argument("Type of the given sub message (which is <" +
        sub_message_type + ">) doesn't match the field type (which is <" +
        message_type + ">)");
  }
  const google::protobuf::Reflection* reflect = m->GetReflection();
  // creates copy of the sub message so that the field doesnt change if you
  // change the message outside
  google::protobuf::Message* copy = messages[sub_message_id]->New(&arena);
  copy->CopyFrom(*messages[sub_message_id]);
  reflect->SetAllocatedMessage(m, copy, fdesc);
}
// makes a COPY of the message and returns its id
int protobuf_schema::get_message(int message_id, int field_tag) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
      -> FindFieldByNumber(field_tag);
  if (fdesc == NULL) {
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
  return insert_message(copy);
}

void protobuf_schema::set_repeated_message(int message_id, int field_tag,
    int sub_message_id, int index = -1) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  if (!(sub_message_id >= 0 && sub_message_id < messages.size())) {
    throw std::out_of_range("No sub message with id: " +
        std::to_string(sub_message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
      -> FindFieldByNumber(field_tag);
  if (fdesc == NULL) {
    throw std::invalid_argument("No field with tag: " +
        std::to_string(field_tag));
  }
  if (fdesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
    throw std::invalid_argument("Field is not message");
  }
  if (!(fdesc->is_repeated())) {
    throw std::invalid_argument("Field is not repeated");
  }
  if (messages[sub_message_id] == NULL || messages[sub_message_id]->
        GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  std::string message_type = fdesc->message_type()->full_name();  // Error
  std::string sub_message_type = messages[sub_message_id]->
      GetDescriptor()->full_name();
  if (message_type.compare(sub_message_type)) {
    throw std::invalid_argument("Type of the given sub message (which is <" +
        sub_message_type + ">) doesn't match the field type (which is <" +
        message_type + ">)");
  }
  const google::protobuf::Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*m, fdesc)) {
    google::protobuf::Message* sub_m = messages[sub_message_id];
    reflect->MutableRepeatedMessage(m, fdesc, index)->CopyFrom(*sub_m);
  } else {
    google::protobuf::Message* copy = messages[sub_message_id]-> New(&arena);
    copy->CopyFrom(*messages[sub_message_id]);
    reflect->AddAllocatedMessage(m, fdesc, copy);
  }
}

// Returns copy of the message
int protobuf_schema::get_repeated_message(int message_id, int field_tag,
      int index) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
      -> FindFieldByNumber(field_tag);
  if (fdesc == NULL) {
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
    return insert_message(copy);
  } else {
    throw std::out_of_range("Index out of range: " + std::to_string(index));
  }
}

void protobuf_schema::set_enum(int message_id, int field_tag, int value) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
     ->FindFieldByNumber(field_tag);
  if (fdesc == NULL) {
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
  if (edesc->FindValueByNumber(value) == NULL) {
    throw std::invalid_argument("Enum doesn't have value: " +
        std::to_string(value));
  }
  m->GetReflection()->SetEnumValue(m, fdesc, value);
}

int protobuf_schema::get_enum(int message_id, int field_tag) {
  if (message_id < 0 || message_id >= messages.size()) {
      throw std::out_of_range("No message with id: " +
          std::to_string(message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
     ->FindFieldByNumber(field_tag);
  if (fdesc == NULL) {
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

void protobuf_schema::set_repeated_enum(int message_id, int field_tag,
    int value, int index = -1) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
     ->FindFieldByNumber(field_tag);
  if (fdesc == NULL) {
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
  if (edesc->FindValueByNumber(value) == NULL) {
    throw std::invalid_argument("Enum doesn't have value: " +
        std::to_string(value));
  }
  const google::protobuf::Reflection* reflect = m->GetReflection();
  if (index >= 0 && index < reflect->FieldSize(*messages[message_id], fdesc)) {
    reflect->SetRepeatedEnumValue(m, fdesc, index, value);
  } else {
    reflect->AddEnumValue(m, fdesc, value);
  }
}

int protobuf_schema::get_repeated_enum(int message_id, int field_tag,
      int index) {
  if (message_id < 0 || message_id >= messages.size()) {
    throw std::out_of_range("No message with id: "
        + std::to_string(message_id));
  }
  google::protobuf::Message* m = messages[message_id];
  if (m == NULL || m->GetDescriptor() == NULL) {
    throw std::runtime_error("Unexpected error");
  }
  const google::protobuf::FieldDescriptor* fdesc = m->GetDescriptor()
      -> FindFieldByNumber(field_tag);
  if (fdesc == NULL) {
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
    const google::protobuf::Message* descriptor,  // Descriptor of the
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
    const google::protobuf::Message* descriptor,  // Descriptor of the erroneous
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
