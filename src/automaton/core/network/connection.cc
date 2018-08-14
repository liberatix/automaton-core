#include "automaton/core/network/connection.h"

namespace automaton {
namespace core {
namespace network {

connection::connection(connection_id id, connection::connection_handler* handler_):
    handler(handler_), id(id) {}

connection* connection::create(const std::string& type, connection_id id, const std::string&
    address, connection::connection_handler* handler) {
  auto it = connection_factory.find(type);
  if (it == connection_factory.end()) {
    return nullptr;
  } else {
    return it -> second(id, address, handler);
  }
}

uint32_t connection::get_id() {
  return id;
}

void connection::register_connection_type(const std::string& type,
    factory_function func) {
  connection_factory[type] = func;
}

std::map<std::string, connection::factory_function>
  connection::connection_factory;

}  // namespace network
}  // namespace core
}  // namespace automaton
