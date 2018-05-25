#ifndef AUTOMATON_CORE_NETWORK_TCP_IMPLEMENTATION_H_
#define AUTOMATON_CORE_NETWORK_TCP_IMPLEMENTATION_H_

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <memory>

#include <boost/asio/io_service.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "automaton/core/network/connection.h"
#include "automaton/core/network/acceptor.h"

namespace automaton {
namespace core {
namespace network {

// TODO(kari): proper exit, clear resources..
// TODO(kari): add worker and init function

/*
  the variables below may be used in handler:
  on_connected() {
    if error && #try < #tries_to_connect
    ...
  }
  on_message_sent() {
    if couldn't sent && #try < #tries_send
      try again  ...
  }
int number_tries_connect = 3;
int number_tries_send = 3;
int number_tries_receive = 3;
int max_connections = 8;

*/

/**
  Class that represents a connection with a remote peer.
*/
class tcp_connection: public connection {
 public:
  /**
    Constructor that will be used when this class is registered.
  */
  tcp_connection(const std::string& address_, connection_handler* handler_);

  /**
    Constructor that will be used from the acceptor.
  */
  tcp_connection(const std::string& address_,
      const boost::asio::ip::tcp::socket& socket_,
      connection_handler* handler_);

  /**
    Destructor.
  */
  ~tcp_connection();

  /**
    This function is used in the constructor. It is used to start async_connect with
    the peer. It can be used outside the constructor if the connection didn't happen
    or was closed or if disconnect() was first used. If a successful connection was
    made, start_listening() and handler's on_connected() will be called.
  */
  void connect();

  /**
    This function is used for sending messages to the peer. It is asynchronous and
    the handler's function on_message_sent() will be called after the message was
    successfully sent or if an error occurred.
  */
  void async_send(const std::string& msg, uint32_t id);

  /**
    If you call this function more than once, events form a queue, no read is
    cancelled
  */
  void async_read(char* buffer, uint32_t buffer_size,
      uint32_t num_bytes, uint32_t id);

  /**
    This function can be called to disconnect peer. To reconnect connect() shoul be
    called. Handler's on_disconnected() is called on successful disconnect,
    on_error(), otherwise.
  */
  void disconnect();

  /**
    This function is used to change the handler. It may be necessary when a
    connection is created from the acceptor and the default handler (the one
    passed to the acceptor) needs to be changed.
  */
  void add_handler(connection_handler* handler_);

  std::string get_address() const;

  connection::state get_state() const;

 private:
  boost::asio::ip::tcp::endpoint asio_endpoint;
  boost::asio::ip::tcp::socket asio_socket;
  std::string address;
};

class tcp_acceptor:public acceptor {
 public:
  /**
    Constructor. Connections_handler will be passed to the connection
    constructor when new connection is accepted and created. It will also be
    used to call its on_connected() method.
  */
  tcp_acceptor(const std::string& address, acceptor_handler* handler_,
      connection::connection_handler* connections_handler);

  /**
    Destructor.
  */
  ~tcp_acceptor();

  /**
    This function is called from the constructor to start asynchronous
    accepting. When a remote peer request to make a connection, handler's
    on_requested() method will be called, passing peer's address and waiting for
    confirmaton to accept or not. If on_requested() returns true, then a
    connection is created and its handler's on_connected() called. Acceptor's
    handler's on_connected will also be called and a pointer to the created
    connection passed. If an error occured while accepting, handler's on_error()
    will be called. TODO(kari): Decide what to do on error.
  */
  void start_accepting();

 private:
  boost::asio::ip::tcp::acceptor asio_acceptor;
  connection::connection_handler* accepted_connections_handler;
};

/**
  For now initializing asio io_service and work objects. New thread is created
  and io_service.run() called in it.
*/
void tcp_init();

void parse_address(const std::string&, std::string* result_addr, std::string*
    result_port);

}  // namespace network
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_NETWORK_TCP_IMPLEMENTATION_H_
