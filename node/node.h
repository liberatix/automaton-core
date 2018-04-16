#ifndef NODE_H__
#define NODE_H__

#include <chrono>  // NOLINT
#include <map>
#include <stack>
#include <string>
#include <vector>

#include "data/schema.h"
#include "network/acceptor.h"
#include "network/connection.h"

class peer;
class node;
class protocol {
 protected:
  std::string id;
  // something like map[peer->state] to store protocol state of peers
  protocol(std::string id_);  // NOLINT
 public:
  ~protocol();
  std::string get_id();
  virtual void process_message(const std::string& message, peer* peer_) = 0;
};

class node {
  //  friend class node_acceptor_handler;
  //  friend class node_connections_handler;
 public:
  /*class node_event_handler {
    // handle events coming from connection and acceptor handlers
    // need event queue, synchronized
    // does NOT exist for now, will be added later.
  };*/
  struct node_options {
    int max_in_connections;
    int max_out_connections;
    int keep_alive_interval;
    int max_buffer;  // must be >= sum(peer.options.buffer_size)
    int used_buffer;
    // ...
  };

  node(const std::string& id, std::vector<protocol*> protocols);
  ~node();
  bool init();
  const std::string& get_id() const;
  // private:
  void add_peer(peer* peer_);
  void remove_peer(std::string id);
  peer* get_peer_by_id(std::string id);
  peer* get_peer_by_connection(connection* c);

  int add_acceptor(acceptor* index);
  void remove_acceptor(int index);

  void add_protocol(protocol* protocol_);
  void remove_protocol(std::string id);

  bool parse_header(const std::string& message, int* size,
      std::string* protocol);
  std::string add_header(const std::string& message,
      const std::string& protocol);
  void send_message(peer* peer_, protocol* protocol,
      const std::string& message);
  void broadcast(protocol* protocol, const std::string& message);

  std::string id;
  // this could be vector with many handlers or COULD NOT EXIST AT ALL
  acceptor::acceptor_handler* acceptor_handler_;
  // this could be vector with many handlers or COULD NOT EXIST AT ALL
  connection::connection_handler* connection_handler_;
  node_options options;
  std::vector<peer*> peers;
  std::map<connection*, int> connection_to_peer;
  std::map<std::string, int> id_to_peer;
  std::vector<acceptor*> acceptors;
  std::map<std::string, protocol*> supported_protocols;
  // std::vector<std::thread> thread_pool;
};

class peer {
  // friend class node;
 public:
  struct peer_options {
    int buffer_size;
    bool is_blocked;
    // ...
  };

  struct peer_stats {
    // all these need to be time instead of int
    int last_connection_attempt;
    int last_connected;
    int last_time_used;
    int last_time_pinged;
    // ...
  };
  peer(const std::string& id, const std::string& address, int buffer_size,
      connection::connection_handler* handler);
  peer(const std::string& id, connection* connection_, int buffer_size);
  ~peer();
  void connect();
  void disconnect();
  void send_message(const std::string& message);
  void async_read(int num_bytes = 0);
  std::string get_id();
  void set_id(const std::string& new_id);
  void add_protocol_state(const std::string& protocol,
      const std::string& state);
  void remove_protocol(const std::string& protocol);
  std::string get_protocol_state(const std::string& protocol);
  connection* peer_connection;
  std::string id;
  peer_options options;
  peer_stats statistics;
  std::map<std::string, std::string> protocol_states;
  bool waiting_header;
  std::string waitng_from_protocol = "";
  int buffer_size;
  char* buffer;
};

class node_connection_handler:public connection::connection_handler {
  node* node_;
 public:
  node_connection_handler(node* node_);  // NOLINT
  ~node_connection_handler();
  void on_message_received(connection* c, const std::string& message);
  void on_message_sent(connection* c, int id, connection::error e);
  void on_connected(connection* c);
  void on_disconnected(connection* c);
  void on_error(connection* c, connection::error e);
};

class node_acceptor_handler:public acceptor::acceptor_handler {
  node* node_;
 public:
  node_acceptor_handler(node* node_);  // NOLINT
  ~node_acceptor_handler();
  bool on_requested(const std::string& address);
  void on_connected(connection* c, const std::string& address);
  void on_error(connection::error e);
};

//////// EXTRA FUNCTIONS ////////

std::string string_to_hex(const std::string& input);
std::string thread_id();

#endif  // NODE_H__
