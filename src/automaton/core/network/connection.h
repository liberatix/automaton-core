#ifndef AUTOMATON_CORE_NETWORK_CONNECTION_H_
#define AUTOMATON_CORE_NETWORK_CONNECTION_H_

#include <map>
#include <string>
#include <thread>

namespace automaton {
namespace core {
namespace network {

// TODO(kari): add state as a member and get_state
// TODO(kari): think about `send and disconnect`

/**
  Class that represents a connection between two peers. It is used to connect to
  remote peer or is used by the acceptor class when accepts an incoming
  connection.
*/

class connection {
 public:
  /**
    TODO(kari)
  */
  enum state {
    invalid_state = 0,
    connecting = 1,
    connected = 2,
    disconnected = 3,
  };

  /**
    TODO(kari)
  */
  enum error {
    no_error = 0,
    unknown = 1,
    invalid_address = 2,
    closed_by_peer = 3,
    connection_refused = 4,
    timed_out = 5,
    broken_pipe = 6,
    not_connected = 7,
  };

  /**
    Handler class used to inform the client for events.
      - on_message_received will be invoked when message from remote peer was
        received; it passes the connection, representing from which peer was the
        message, and the message that was received.
      - on_message_sent will be invoked when a message was sent successfully
        using async_send.
      - on_connected will be invoked when the connection with the remote peer
        was established.
      - on_disconnected will be invoked when connection was closed/ destroyed
        by the local or the remote peer
      - on_error will be invoked when an error happens while listening or
        accepting.
  */
  class connection_handler {
   public:
    virtual void on_message_received(connection* c, char* buffer,
        unsigned int bytes_read, unsigned int id) = 0;
    virtual void on_message_sent(connection* c, unsigned int id,
        connection::error e) = 0;
    virtual void on_connected(connection* c) = 0;
    virtual void on_disconnected(connection* c) = 0;
    virtual void on_error(connection* c, connection::error e) = 0;
  };

  /**
    Function that is used to send message to the remote peer. Id shows the
    sequence_id at the time of sending the message. On_message_sent should be
    invoked once the message was sent successfully.
  */
  virtual void async_send(const std::string& message, unsigned int id = 0) = 0;
  virtual void async_read(char* buffer, unsigned int buffer_size,
      unsigned int num_bytes = 0, unsigned int id = 0) = 0;

  virtual state get_state() const = 0;
  virtual std::string get_address() const = 0;
  virtual void connect() = 0;
  virtual void disconnect() = 0;

  /**
    Function that is used to create objects from a specified child class.
    The child class should first be registered using register_acceptor_type
    function. The function returns object from the specified class. If no such
    class type was registered, NULL will be returned.
  */
  static connection* create(const std::string& type, const std::string& address,
      connection_handler* handler);

  typedef connection* (*factory_function)(const std::string& address,
      connection_handler* handler);

  /**
    Function that is used to register how an object from child class will be
    created. Type shows how the class will be referenced (ex. "child_class_1"),
    factory_function specifies the function tht will be used to create an
    object. See "typedef connection* (*factory_function)..." above to check what
    arguments should this function accept. If such type name exists in the
    registry, the factory_function pointer will be overriden.
  */
  static void register_connection_type(const std::string& type,
      factory_function func);

 protected:
  /**
  Class constructor.
  */
  explicit connection(connection_handler* handler_);

  /**
    Handler object that must be set so the client could be informed for events.
    If no handler or a handler with empty function implementations is provided,
    client will not have access to events information like connect/ disconnect,
    received messages or an error that happend.
  */
  connection_handler* handler;

 private:
  /**
    Map that stores registered child classes and pointers to funtions that are
    used to create objects.
  */
  static std::map<std::string, factory_function> connection_factory;
};

}  // namespace network
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_NETWORK_CONNECTION_H_
