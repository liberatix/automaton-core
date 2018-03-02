#include "network/tcp_implementation.h"

#include <regex>
#include <thread>
#include <mutex>

// TODO(kari): proper exit, clear resources..
// TODO(kari): improve init function and handle the new thread
// TODO(kari): Add "if ! initialized" where necessary

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

boost::asio::io_service asio_io_service;
boost::asio::io_service::work _work(asio_io_service);
std::thread* worker_thread;
bool initialized = false;

// Connection functions

tcp_connection::tcp_connection(const std::string& _address, connection_handler*
    _handler):connection(_handler), asio_socket{asio_io_service} {
  address = _address;
  boost::system::error_code boost_error_code;
  boost::asio::ip::tcp::resolver resolver{asio_io_service};
  std::string ip, port;
  parse_address(_address, &ip, &port);
  boost::asio::ip::tcp::resolver::query q{ip, port};
  asio_endpoint = *resolver.resolve(q, boost_error_code);
  if (boost_error_code) {
    logging("ERROR 6: " + boost_error_code.message());
    _handler->on_error(this, connection::error::invalid_address);
  } else {
    connect();
  }
}

tcp_connection::tcp_connection(const std::string& addr, const
    boost::asio::ip::tcp::socket& sock, connection_handler* _handler):
    connection(_handler),
    asio_socket(std::move(const_cast<boost::asio::ip::tcp::socket&>(sock))),
    address(addr) {
  start_listening();
}

tcp_connection::~tcp_connection() {
  boost::system::error_code boost_error_code;
  asio_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
      boost_error_code);
  if (boost_error_code) {
    logging("ERROR while destroying connection: " +
        boost_error_code.message());
  }
  asio_socket.close();
}

void tcp_connection::connect() {
  asio_socket.async_connect(asio_endpoint,
      [this](const boost::system::error_code& boost_error_code) {
        if (boost_error_code) {
          logging("ERROR 5: " + boost_error_code.message());
          handler->on_error(this, connection::error::unknown);
        } else {
          handler->on_connected(this);
          start_listening();
        }
    });
}

void tcp_connection::async_send(const std::string& msg, int id) {
  if (asio_socket.is_open()) {
    std::string* message = new std::string(msg);
    asio_socket.async_write_some(boost::asio::buffer(*message),
        [this, id, message](const boost::system::error_code& boost_error_code,
        size_t bytes_transferred) {
      if (boost_error_code) {
        logging("ERROR 2: " + boost_error_code.message());
        handler->on_message_sent(this, id, connection::error::unknown);
       // if (bytes_transferred < message.size())
      } else {
        handler->on_message_sent(this, id, connection::error::no_error);
      }
      delete message;
    });
  } else {
    // TODO(kari): what to do here? needs to be connected
  }
}

void tcp_connection::start_listening() {
  if (asio_socket.is_open()) {
    asio_socket.async_read_some(boost::asio::buffer(msg_buffer, BUFFER_SIZE),
        [this](const boost::system::error_code& boost_error_code,
        size_t bytes_transferred) {
      if (boost_error_code) {
        if (boost_error_code == boost::asio::error::eof) {
          handler->on_disconnected(this);
          return;
        } else if (boost_error_code == boost::asio::error::operation_aborted) {
          return;
        } else {
          logging("ERROR 3: " + boost_error_code.message());
          handler->on_error(this, connection::error::unknown);
          // TODO(kari): what errors and when should start_listening be called?
        }
      } else {
        handler->on_message_received(this, std::string(msg_buffer, 0,
            bytes_transferred));
      }
      start_listening();
    });
  } else {
    // TODO(kari): what to do here? needs to be connected
  }
}

void tcp_connection::disconnect() {
  boost::system::error_code boost_error_code;
  asio_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
      boost_error_code);
  if (boost_error_code) {
    handler->on_error(this, connection::error::unknown);
    logging("ERROR 7: " + boost_error_code.message());
  } else {
    asio_socket.close();
    handler->on_disconnected(this);
  }
}

void tcp_connection::add_handler(connection_handler* _handler) {
  this->handler = _handler;
}

std::string tcp_connection::get_address() {
  return address;
}

// Acceptor functions

tcp_acceptor::tcp_acceptor(const std::string& address, acceptor_handler*
    _handler, connection::connection_handler* connections_handler):
    acceptor(_handler), asio_acceptor{asio_io_service},
    accepted_connections_handler(connections_handler) {
  boost::asio::ip::tcp::resolver resolver{asio_io_service};
  boost::system::error_code boost_error_code;
  std::string ip,port;
  parse_address(address, &ip, &port);
  boost::asio::ip::tcp::resolver::query q{ip, port};
  boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(q,
      boost_error_code);
  if (boost_error_code) {
    logging("Error 0: " + boost_error_code.message());
    handler->on_error(connection::error::invalid_address);
  } else {
    boost::system::error_code ecc;
    asio_acceptor.open(((boost::asio::ip::tcp::endpoint)*it).protocol());
    asio_acceptor.bind(*it, ecc);
    if (ecc) {
      handler->on_error(connection::error::unknown);
      logging("Error 1: " + ecc.message());
    } else {
      asio_acceptor.listen();
      start_accepting();
    }
  }
}

tcp_acceptor::~tcp_acceptor() {
  asio_acceptor.close();
}

void tcp_acceptor::start_accepting() {
  // tcp_connection* new_connection = new tcp_connection();
  asio_acceptor.async_accept(asio_io_service, [this]
      (const boost::system::error_code& boost_error_code,
      boost::asio::ip::tcp::socket _socket) {
     if (!boost_error_code) {
        boost::asio::ip::tcp::endpoint remote_endpoint =
            _socket.remote_endpoint();
        std::string remote_address = (remote_endpoint.address()).to_string() +
        ":" + std::to_string(remote_endpoint.port());
        bool accepted = handler->on_requested(remote_address);
        if (accepted) {
          tcp_connection* new_con = new tcp_connection(remote_address, _socket,
              accepted_connections_handler);
          handler->on_connected(new_con, remote_address);
          if (accepted_connections_handler) {
            accepted_connections_handler->on_connected(new_con);
          }
        }
        start_accepting();
      } else {
        // TODO(kari): Handle errors
        logging("ERROR 10: " + boost_error_code.message());
        handler->on_error(connection::error::unknown);
        // TODO(kari): start listen again? depends on the errors
        // start_accepting();
      }
    });
}

// Global functions

void start() {
  asio_io_service.run();
}

void tcp_init() {
  initialized = true;
  worker_thread = new std::thread(start);
}

std::mutex console_mutex;
void logging(const std::string& s) {
  console_mutex.lock();
  std::cout << s << std::endl;
  console_mutex.unlock();
}

// void destroy(){}; stop work, join thread, clean resources...

void parse_address(const std::string& address, std::string* result_addr,
  std::string* result_port) {
  std::regex rgx_ip("((?:\\d+\\.)+\\d+|(?:[0-9a-f]+:)+[0-9a-f]+):(\\d+)");
  std::smatch match;
  if (std::regex_match(address.begin(), address.end(), match, rgx_ip) &&
      match.size() == 3) {
    *result_addr = match[1];
    *result_port = match[2];
  } else {
    *result_addr = "";
    *result_port = "";
  }
}
