#include "acceptor.h"

acceptor::acceptor(acceptor::acceptor_handler* _handler):handler(_handler) {}

acceptor* acceptor::create(const std::string& type,
    const std::string& addr, const std::string& port, 
    acceptor::acceptor_handler* _handler, connection::connection_handler* 
    connections_handler) {
  auto it = acceptor_factory.find(type);
  if (it == acceptor_factory.end()) {
    return NULL;
  }
  else {
    return it -> second(addr, port, _handler, connections_handler);
  }
}
void acceptor::register_acceptor_type(
    const std::string& type, factory_function func) {
  acceptor_factory[type] = func;
}

std::map<std::string, acceptor::factory_function>
    acceptor::acceptor_factory;

