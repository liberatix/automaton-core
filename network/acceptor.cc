#include "network/acceptor.h"

acceptor::acceptor(acceptor::acceptor_handler* handler_):handler(handler_) {}

acceptor* acceptor::create(const std::string& type,
    const std::string& address, acceptor::acceptor_handler* handler_,
    connection::connection_handler* connections_handler) {
  auto it = acceptor_factory.find(type);
  if (it == acceptor_factory.end()) {
    return NULL;
  } else {
    return it -> second(address, handler_, connections_handler);
  }
}
void acceptor::register_acceptor_type(
    const std::string& type, factory_function func) {
  acceptor_factory[type] = func;
}

std::map<std::string, acceptor::factory_function>
    acceptor::acceptor_factory;
