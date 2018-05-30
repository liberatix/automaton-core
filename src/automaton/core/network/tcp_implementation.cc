#include "automaton/core/network/tcp_implementation.h"

#include <mutex>
#include <regex>
#include <thread>

#include <boost/asio/read.hpp>

#include "automaton/core/log/log.h"

namespace automaton {
namespace core {
namespace network {

// TODO(kari): proper exit, clear resources..
// TODO(kari): improve init function and handle the new thread
// TODO(kari): Add "if ! tcp_initialized" where necessary

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

static boost::asio::io_service asio_io_service;
static boost::asio::io_service::work work_(asio_io_service);
static std::thread* worker_thread;
static bool tcp_initialized = false;

// Connection functions

tcp_connection::tcp_connection(const std::string& address_, connection_handler*
    handler_):connection(handler_), asio_socket{asio_io_service} {
  address = address_;
  boost::system::error_code boost_error_code;
  boost::asio::ip::tcp::resolver resolver{asio_io_service};
  std::string ip, port;
  parse_address(address_, &ip, &port);
  boost::asio::ip::tcp::resolver::query q{ip, port};
  asio_endpoint = *resolver.resolve(q, boost_error_code);
  if (boost_error_code) {
    LOG(ERROR) << boost_error_code.message();
    handler_->on_error(this, connection::error::invalid_address);
  } else {
    // LOG(INFO) << "Successfully resolved address, no connection was made yet";
  }
}

tcp_connection::tcp_connection(const std::string& addr, const
    boost::asio::ip::tcp::socket& sock, connection_handler* handler_):
    connection(handler_),
    asio_socket(std::move(const_cast<boost::asio::ip::tcp::socket&>(sock))),
    address(addr) {
}

tcp_connection::~tcp_connection() {
  boost::system::error_code boost_error_code;
  asio_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
      boost_error_code);
  if (boost_error_code) {
    LOG(ERROR) << boost_error_code.message();
  }
  asio_socket.close();
}

void tcp_connection::connect() {
  if (tcp_initialized) {
    // TODO(kari): if already connected {}
    asio_socket.async_connect(asio_endpoint,
        [this](const boost::system::error_code& boost_error_code) {
      if (boost_error_code) {
          LOG(ERROR) << boost_error_code.message();
          if (boost_error_code == boost::asio::error::connection_refused) {
            handler->on_error(this, connection::error::connection_refused);
          } else {
            handler->on_error(this, connection::error::unknown);
          }
        } else {
          handler->on_connected(this);
        }
    });
  } else {
    LOG(ERROR) << "Not initialized! tcp_init() must be called first";
    handler->on_error(this, connection::error::unknown);
    // TODO(kari): what to do here? needs to be connected
  }
}

void tcp_connection::async_send(const std::string& msg, uint32_t id) {
  if (tcp_initialized && asio_socket.is_open() && msg.size() > 0) {
    std::string* message = new std::string(msg);
    asio_socket.async_write_some(boost::asio::buffer(*message),
        [this, id, message](const boost::system::error_code& boost_error_code,
        size_t bytes_transferred) {
      if (boost_error_code) {
        LOG(ERROR) << boost_error_code.message();
        if (boost_error_code == boost::asio::error::broken_pipe) {
          handler->on_message_sent(this, id, connection::error::closed_by_peer);
          // TODO(kari): ?? handle
        } else {
          handler->on_message_sent(this, id, connection::error::unknown);
        }
       // if (bytes_transferred < message.size())
      } else {
        handler->on_message_sent(this, id, connection::error::no_error);
      }
      delete message;
    });
  } else if (!tcp_initialized) {
    LOG(ERROR) << "Not initialized";
    // handler->on_error(this, connection::error::unknown);
    handler->on_message_sent(this, id, connection::error::unknown);
    // TODO(kari): what to do here? needs to be connected
  } else {
    LOG(ERROR) << "Socket closed";
    // handler->on_error(this, connection::error::closed_by_peer);
    handler->on_message_sent(this, id, connection::error::closed_by_peer);
  }
}

void tcp_connection::async_read(char* buffer, uint32_t buffer_size,
    uint32_t num_bytes, uint32_t id) {
  if (tcp_initialized && asio_socket.is_open()) {
    if (num_bytes <= 0) {
      asio_socket.async_read_some(boost::asio::buffer(buffer, buffer_size),
          [this, buffer, id](const boost::system::error_code& boost_error_code,
          size_t bytes_transferred) {
        if (boost_error_code) {
          if (boost_error_code == boost::asio::error::eof) {
            handler->on_disconnected(this);
            return;
          } else if (boost_error_code ==
                boost::asio::error::operation_aborted) {
            return;
          } else {
            LOG(ERROR) << boost_error_code.message();
            handler->on_error(this, connection::error::unknown);
            // TODO(kari): what errors and when should read be called?
          }
        } else {
          handler->on_message_received(this, buffer, bytes_transferred, id);
        }
      });
    } else {
        boost::asio::async_read(asio_socket,
          boost::asio::buffer(buffer, buffer_size),
          boost::asio::transfer_exactly(num_bytes),
          [this, buffer, id](const boost::system::error_code& boost_error_code,
          size_t bytes_transferred) {
        if (boost_error_code) {
          if (boost_error_code == boost::asio::error::eof) {
            handler->on_disconnected(this);
            return;
          } else if (boost_error_code ==
              boost::asio::error::operation_aborted) {
            return;
          } else {
            LOG(ERROR) << boost_error_code.message();
            handler->on_error(this, connection::error::unknown);
            // TODO(kari): what errors and when should read be called?
          }
        } else {
          handler->on_message_received(this, buffer, bytes_transferred, id);
        }
      });
    }
  } else if (!tcp_initialized) {
    LOG(ERROR) << "Not initialized";
    handler->on_error(this, connection::error::unknown);
    // TODO(kari): what to do here? needs to be connected
  } else {
    LOG(ERROR) << "Socket closed";
    handler->on_error(this, connection::error::closed_by_peer);
  }
}

void tcp_connection::disconnect() {
  if (asio_socket.is_open()) {
    boost::system::error_code boost_error_code_shut;
    asio_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
        boost_error_code_shut);
    if (boost_error_code_shut) {
      handler->on_error(this, connection::error::unknown);
      LOG(ERROR) << boost_error_code_shut.message();
    }
    boost::system::error_code boost_error_code_close;
    asio_socket.close(boost_error_code_close);
    if (boost_error_code_close) {
      handler->on_error(this, connection::error::unknown);
      LOG(ERROR) << boost_error_code_close.message();
    }
  }
  handler->on_disconnected(this);
}

void tcp_connection::add_handler(connection_handler* handler_) {
  this->handler = handler_;
}

std::string tcp_connection::get_address() const {
  return address;
}

connection::state tcp_connection::get_state() const {
  // TODO(kari): implement this
  return connection::state::invalid_state;
}

// Acceptor functions

tcp_acceptor::tcp_acceptor(const std::string& address, acceptor_handler*
    handler_, connection::connection_handler* connections_handler_):
    acceptor(handler_), asio_acceptor{asio_io_service},
    accepted_connections_handler(connections_handler_) {
  if (!tcp_initialized) {
    std::stringstream msg;
    msg << "TCP is not initialized! Call tcp_init() first!";
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    throw std::runtime_error(msg.str());
  }
  boost::asio::ip::tcp::resolver resolver{asio_io_service};
  boost::system::error_code boost_error_code;
  std::string ip, port;
  parse_address(address, &ip, &port);
  boost::asio::ip::tcp::resolver::query q{ip, port};
  boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(q,
      boost_error_code);
  if (boost_error_code) {
    LOG(ERROR) << boost_error_code.message();
    handler->on_error(connection::error::invalid_address);
  } else {
    boost::system::error_code ecc;
    asio_acceptor.open(((boost::asio::ip::tcp::endpoint)*it).protocol());
    asio_acceptor.bind(*it, ecc);
    if (ecc) {
      handler->on_error(connection::error::unknown);
      LOG(ERROR) << ecc.message();
    } else {
      asio_acceptor.listen();
    }
  }
}

tcp_acceptor::~tcp_acceptor() {
  asio_acceptor.close();
}

void tcp_acceptor::start_accepting() {
  if (tcp_initialized && asio_acceptor.is_open()) {
    asio_acceptor.async_accept(asio_io_service, [this]
        (const boost::system::error_code& boost_error_code,
        boost::asio::ip::tcp::socket socket_) {
       if (!boost_error_code) {
          boost::asio::ip::tcp::endpoint remote_endpoint =
              socket_.remote_endpoint();
          std::string remote_address = (remote_endpoint.address()).to_string() +
          ":" + std::to_string(remote_endpoint.port());
          bool accepted = handler->on_requested(remote_address);
          if (accepted) {
            tcp_connection* new_con = new tcp_connection(remote_address,
                socket_, accepted_connections_handler);
            handler->on_connected(new_con, remote_address);
            if (accepted_connections_handler) {
              accepted_connections_handler->on_connected(new_con);
            }
          }
          start_accepting();
        } else {
          // TODO(kari): Handle errors
          LOG(ERROR) << boost_error_code.message();
          handler->on_error(connection::error::unknown);
          // TODO(kari): start listen again? depends on the errors
          // start_accepting();
        }
      });
  } else if (!tcp_initialized) {
    LOG(ERROR) << "Not initialized";
    handler->on_error(connection::error::unknown);
    // TODO(kari): what to do here? needs to be connected
  } else {
    LOG(ERROR) << "Acceptor closed";
    handler->on_error(connection::error::closed_by_peer);
  }
}

// Global functions

void tcp_init() {
  if (tcp_initialized) {
    return;
  }
  connection::register_connection_type("tcp", [](const std::string& address,
    connection::connection_handler* handler) {
    return reinterpret_cast<connection*>(new tcp_connection(address, handler));
  });
  acceptor::register_acceptor_type("tcp", [](const std::string& address,
      acceptor::acceptor_handler* handler_, connection::connection_handler*
      connections_handler_) {
    return reinterpret_cast<acceptor*>(new tcp_acceptor(address, handler_,
      connections_handler_));
  });
  worker_thread = new std::thread([]() {
      asio_io_service.run();
  });
  tcp_initialized = true;
}

// TODO(kari): void destroy(){}; stop work, join thread, clean resources...

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

}  // namespace network
}  // namespace core
}  // namespace automaton
