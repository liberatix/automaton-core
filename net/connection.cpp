#include "connection.h"

connection::connection(connection::connection_handler* _handler):
    handler(_handler) {};
connection* connection::create(const std::string& type, const std::string& 
    address, const std::string& port, connection::connection_handler* handler){
  auto it = connection_factory.find(type);
  if (it == connection_factory.end()) {
    return nullptr;
  }
  else {
    return it -> second(address, port, handler);
  }
}

void connection::register_connection_type(const std::string& type, 
    factory_function func) {
  connection_factory[type] = func;
}

int connection::get_next_sequence_id(){
  return sequence_id++;
} 

std::map<std::string, connection::factory_function> 
  connection::connection_factory;
