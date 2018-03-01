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

boost::asio::io_service _io_service;
boost::asio::io_service::work _work(_io_service);
bool initialized = false;

// Connection functions

tcp_connection::tcp_connection(const std::string& _address, const std::string&
    _port, connection_handler* _handler):connection(_handler),
    _socket{_io_service} {
  // TODO(kari): parse from "adress:port" to adress and port (ip4 & ip6) and
  // change the constructor
  address = _address + ":" + _port;
  boost::asio::ip::tcp::resolver resolver{_io_service};
  boost::asio::ip::tcp::resolver::query q{_address, _port};
  _endpoint = *resolver.resolve(q, ec);
  if (ec) {
    logging("ERROR 6: " + ec.message());
    _handler->on_error(this, connection::error::invalid_address);
  } else {
    connect();
  }
}

tcp_connection::tcp_connection(const std::string& addr, const
    boost::asio::ip::tcp::socket& sock, connection_handler* _handler):
    connection(_handler),
    _socket(std::move(const_cast<boost::asio::ip::tcp::socket&>(sock))),
    address(addr) {
  start_listening();
}

tcp_connection::~tcp_connection() {
  boost::system::error_code ec;
  _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
  if (ec) {
     logging("ERROR while destroying connection: " + ec.message());
  }
  _socket.close();
}

void tcp_connection::connect() {
  _socket.async_connect(_endpoint,
      [this](const boost::system::error_code& ec) {
        if (ec) {
          logging("ERROR 5: " + ec.message());
          handler->on_error(this, connection::error::unknown);
        } else {
          handler->on_connected(this);
          start_listening();
        }
    });
}

void tcp_connection::async_send(const std::string& msg, int id) {
  if (_socket.is_open()) {
    std::string* message = new std::string(msg);
    _socket.async_write_some(boost::asio::buffer(*message), [this, id, message]
        (const boost::system::error_code& ec, size_t bytes_transferred) {
      if (ec) {
        logging("ERROR 2: " + ec.message());
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
  if (_socket.is_open()) {
    _socket.async_read_some(boost::asio::buffer(msg_buffer, 256), [this]
        (const boost::system::error_code& ec, size_t bytes_transferred) {
      if (ec) {
        if (ec == boost::asio::error::eof) {
          handler->on_disconnected(this);
          return;
        } else if (ec == boost::asio::error::operation_aborted) {
          return;
        } else {
          logging("ERROR 3: " + ec.message());
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
  boost::system::error_code ec;
  _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
  if (ec) {
    handler->on_error(this, connection::error::unknown);
    logging("ERROR 7: " + ec.message());
  } else {
    _socket.close();
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

tcp_acceptor::tcp_acceptor(const std::string& addr, const std::string& port,
    acceptor_handler* _handler, connection::connection_handler*
    connections_handler):acceptor(_handler), _acceptor{_io_service},
    accepted_connections_handler(connections_handler) {
  boost::asio::ip::tcp::resolver resolver{_io_service};
  boost::system::error_code ec;
  boost::asio::ip::tcp::resolver::query q{addr, port};
  boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(q, ec);
  if (ec) {
    logging("Error 0: " + ec.message());
    handler->on_error(connection::error::invalid_address);
  } else {
    boost::system::error_code ecc;
    _acceptor.open(((boost::asio::ip::tcp::endpoint)*it).protocol());
    _acceptor.bind(*it, ecc);
    if (ecc) {
      handler->on_error(connection::error::unknown);
      logging("Error 1: " + ecc.message());
    } else {
      _acceptor.listen();
      start_accepting();
    }
  }
}

tcp_acceptor::~tcp_acceptor() {
  _acceptor.close();
}

void tcp_acceptor::start_accepting() {
  // tcp_connection* new_connection = new tcp_connection();
  _acceptor.async_accept(_io_service, [this]
      (const boost::system::error_code& ec, boost::asio::ip::tcp::socket
      _socket) {
     if (!ec) {
        boost::asio::ip::tcp::endpoint remote_endpoint =
            _socket.remote_endpoint();
        std::string remote_address = (remote_endpoint.address()).to_string() +
        ":" + std::to_string(remote_endpoint.port());
        bool accepted = handler->on_requested(remote_address);
        if (accepted) {
          tcp_connection* new_con = new tcp_connection(remote_address, _socket,
              accepted_connections_handler);
          handler->on_connected(new_con);
          if (accepted_connections_handler) {
            accepted_connections_handler->on_connected(new_con);
          }
        }
        start_accepting();
      } else {
        // TODO(kari): Handle errors
        logging("ERROR 10: " + ec.message());
        handler->on_error(connection::error::unknown);
        // TODO(kari): start listen again? depends on the errors
        // start_accepting();
      }
    });
}

// Global functions

void start() {
  _io_service.run();
}

void tcp_init() {
  initialized = true;
  std::thread* t = new std::thread(start);
}

std::mutex console_mutex;
void logging(const std::string& s) {
  console_mutex.lock();
  std::cout << s << std::endl;
  console_mutex.unlock();
}

// void destroy(){}; stop work, join thread, clean resources...
/*
void parse_address(const std::string& address, std::string& result_addr,
    std::string& result_port) {
    std::regex rgx("([\\d+\\.]+\\d+|[\\d+\\:]+\\d+):(\\d+)");
    std::smatch match;
    if (std::regex_match(address.begin(), address.end(), match, rgx) &&
        match.size() == 3) {
      result_addr = match[1];
      result_port = match[2];
    }
}*/
