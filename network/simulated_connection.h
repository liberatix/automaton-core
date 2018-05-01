#ifndef NETWORK_SIMULATED_CONNECTION_H__
#define NETWORK_SIMULATED_CONNECTION_H__

#include <mutex>
#include <string>
#include <queue>
#include <vector>

#include "network/acceptor.h"
#include "network/connection.h"

void simulation_init();
/**
  1. Peer A makes connection attemp to peer B when connect() is called.
  2. Peer B can accept or refuse
  ...
**/
enum event_type {
  default_event = 0,  //
  disconnect = 1,  // this is created when disconnect is called and handled on
  // the other end after some lag
  connection_attempt = 2,  // this is created when connect is called and handled
  // on the other end after some lag; on_requested is called
  send = 3,  // this is created when send is called and handled after some lag;
  // when handling send on_message_sent is called and read events are generated
  // for the other end
  read = 4,  // a message need to be read and on_message_received to be called
  // if no read has been called, event time is increased
  accept = 5,  // this is created when a connection is accepted (on_requested
  // returns true) and handled on the other end afted some lag
  refuse = 6,  // as above
  error = 7
};

class simulated_connection;
class simulated_acceptor;
// this could be protobuf message
// TODO(kari): Shrink it
struct event {
  static unsigned int event_ids;
  unsigned int event_id;
  event_type type;
  unsigned int time_created;  // not used right now
  unsigned int time_of_handling;  // used for sorting in the event queue
  simulated_connection* connection_;
  simulated_acceptor* acceptor_;
  std::string data;
  unsigned int id;
  event();
  std::string to_string() const;
};

// this could be protobuf message
struct connection_params {
  unsigned int min_lag;
  unsigned int max_lag;
  unsigned int max_connections;
  unsigned int bandwidth;
};

class simulation {
 private:
  struct q_comperator {
    bool operator() (const event& lhs, const event& rhs) const;
  };

  std::priority_queue<event, std::vector<event>, q_comperator> event_q;
  std::mutex q_mutex;
  bool quit_pressed;
  unsigned int simulation_time;
  std::thread input_thread;
  std::thread simulation_thread;
  static simulation* simulator;  // instance
  simulation();
  void input();
  void handle_event(const event& event_);
  void loop();  // void loop(void (*handle_event)(const event& event));
 public:
  ~simulation();
  static simulation* get_simulator();
  bool quit();
  void push_event(const event& event_);
  unsigned int time();
};

class simulated_connection: public connection {
 public:
  int address_from;
  int address_to;
  state connection_state;
  connection_params parameters;

  // Next 5 are used for read event
  std::queue<char*> buffers;
  std::queue<unsigned int> buffers_sizes;
  std::queue<unsigned int> expect_to_read;
  std::queue<int> read_ids;
  unsigned int bytes_read;

  simulated_connection(const std::string& address_,
      connection_handler* handler_);
  bool parse_address(const std::string& address);

  void async_send(const std::string& message, unsigned int message_id);
  void async_read(char* buffer, unsigned int buffer_size,
      unsigned int num_bytes, unsigned int id);

  state get_state() const;
  std::string get_address() const;
  void connect();
  void disconnect();
  connection_handler* get_handler();
};

class simulated_acceptor: public acceptor {
 public:
  std::string address;
  bool started_accepting;
  connection::connection_handler* accepted_connections_handler;
  simulated_acceptor(const std::string& address_, acceptor::acceptor_handler*
      handler_, connection::connection_handler* accepted_connections_handler);

  void start_accepting();
  acceptor_handler* get_handler();
};

void logging(const std::string& s);

#endif  // NETWORK_SIMULATED_CONNECTION_H__
