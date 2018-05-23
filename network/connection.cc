#include "network/connection.h"

namespace automaton {
namespace core {
namespace network {

connection::connection(connection::connection_handler* handler_):
    handler(handler_) {}

connection* connection::create(const std::string& type, const std::string&
    address, connection::connection_handler* handler) {
  auto it = connection_factory.find(type);
  if (it == connection_factory.end()) {
    return nullptr;
  } else {
    return it -> second(address, handler);
  }
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
