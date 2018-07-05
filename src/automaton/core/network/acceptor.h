#ifndef AUTOMATON_CORE_NETWORK_ACCEPTOR_H_
#define AUTOMATON_CORE_NETWORK_ACCEPTOR_H_

#include <map>
#include <string>

#include "automaton/core/network/connection.h"

namespace automaton {
namespace core {
namespace network {

// Class that is used to listen for and accept incoming connections. (Server)
class acceptor {
 public:
  enum state {
    invalid_state = 0,
    accepting = 1,
    not_accepting = 2,
  };
  /**
    Handler class used to inform the client for events.
      - on_requested will be invoked when a peer wants to connect; it passes
        the address of the remote peer.
      - on_connected will be invoked if the connection was accepted by the
        peer and connection was made; it passes the resulting connection or null
        if no connection was made;
      - on_error will be invoked when an error happens while listening or
        accepting.
  */
  class acceptor_handler {
   public:
    // IDEA(kari): return string (schema message with connection params) instead
    // of bool
    virtual bool on_requested(acceptor* a, const std::string& address) = 0;
    virtual void on_connected(acceptor* a, connection* c, const std::string& address) = 0;
    virtual void on_error(acceptor* a, connection::error e) = 0;
  };
  virtual ~acceptor() {}

  virtual bool init() = 0;

  /**
    Function that defines how the acceptor works. It should specify the way
    acceptor listens for and accepts connections. It should call handler's
    functions on the specified events to inform the client about them.
  */
  virtual void start_accepting() = 0;

  virtual std::string get_address() const = 0;

  virtual acceptor::state get_state() = 0;

  /**
    Function that is used to create objects from a specified child class.
    The child class should first be registered using register_acceptor_type
    function. The function returns object from the specified class. If no such
    class type was registered, NULL will be returned.
  */
  static acceptor* create(const std::string& type, const std::string& address,
      acceptor_handler* handler_, connection::connection_handler*
      connections_handler);

  typedef acceptor* (*factory_function)(const std::string& address, acceptor_handler* handler_,
      connection::connection_handler* connections_handler);

  /**
    Function that is used to register how an object from child class will be
    created. Type shows how the class will be referenced (ex. "child_class_1"),
    factory_function specifies the function tht will be used to create an
    object. See "typedef acceptor* (*factory_function)..." above to check what
    arguments should this function accept. If such type name exists in the
    registry, the factory_function pointer will be overriden.
  */
  static void register_acceptor_type(const std::string& type, factory_function func);

 protected:
  /** Class constructor. */
  explicit acceptor(acceptor_handler* handler_);

  /**
    Handler object that must be set so the client could be informed for events.
    If no handler or a handler with empty function implementations is provided,
    client will not have access to events information like connection request or
    connection that was made or an error that happend.
  */
  acceptor_handler* handler;

 private:
  /**
    Map that stores registered child classes and pointers to funtions that are
    used to create objects.
  */
  static std::map<std::string, factory_function> acceptor_factory;
};

}  // namespace network
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_NETWORK_ACCEPTOR_H_
