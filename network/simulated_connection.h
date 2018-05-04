#ifndef NETWORK_SIMULATED_CONNECTION_H__
#define NETWORK_SIMULATED_CONNECTION_H__

#include <map>
#include <mutex>
#include <string>
#include <queue>
#include <vector>
#include <utility>

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
  static unsigned int new_id();  // gives unique event id

  unsigned int event_id;
  event_type type;
  // unsigned int time_created;
  uint64_t time_of_handling;  // used for sorting in the event queue;
  simulated_connection* connection_;
  simulated_acceptor* acceptor_;
  std::string data;
  unsigned int id;  // used for sorting in the event queue;
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

/**
  Singleton class running the simulation. Stores created acceptors,
  connections, events and simulation time.
**/
class simulation {
 private:
  /**
   Comparator for the event queue -> events with lower time come first, if
   time is equal, event id is used and events created first are handled first.
  **/
  struct q_comparator {
    bool operator() (const event& lhs, const event& rhs) const;
  };

  // TODO(kari): remove this
  /*class map_comparator {
   public:
    bool operator() (const std::pair<int, int>& lhs,
        const std::pair<int, int>& rhs) const {
      if (lhs.first == rhs.first) {
        return lhs.second < rhs.second;
      }
      return lhs.first < rhs.first;
    }
  };*/

  /**
    Map storing created connections where the key is <address_A, address_B> and
    the value represents connection A->B. B->A is stored separately.
  **/
  std::map<std::pair<int, int>, simulated_connection*> connections;
  std::mutex connections_mutex;
  /**
    Map storing created acceptors where the key is the address.
  **/
  std::map<int, simulated_acceptor*> acceptors;
  std::mutex acceptors_mutex;
  /**
    Priority queue storing the events that need to be handled. Lower time of
    handlig means higher priority. If equal, lower event id (created earlier)
    means higher priority.
  **/
  std::priority_queue<event, std::vector<event>, q_comparator> event_q;
  std::mutex q_mutex;
  /**
    Simulation time. On create is 0.
  **/
  uint64_t simulation_time;
  std::mutex time_mutex;
  /**
    The simulation instance.
  **/
  static simulation* simulator;
  // Constructor.
  simulation();
  /**
    Function that handles the events from the queue. It is called from
    process().
  **/
  void handle_event(const event& event_);
  /**
    Update the simulation time.
  **/
  void set_time(uint64_t time);

 public:
  ~simulation();
  /**
    Returns pointer to the simulation instance;
  **/
  static simulation* get_simulator();
  /**
    Add event to the event queue. It is called from connection
    functions(connect, send, etc.) or from handle_event
  **/
  void push_event(const event& event_);
  // Returns current simulation time
  uint64_t get_time();
  /**
    Process all events from simulation_time to the given time.
  **/
  void process(uint64_t time);
  void add_connection(int address_a, int address_b,
      simulated_connection* connection_);
  void add_acceptor(int address, simulated_acceptor* acceptor_);
  simulated_connection* get_connection(int address_a, int address_b);
  simulated_acceptor* get_acceptor(int address);
  // DEBUG
  void print_q();
  void print_connections();
};

class simulated_connection: public connection {
 public:
  int address_from;
  int address_to;
  state connection_state;
  unsigned int last_event;  // this is used when setting event time to prevent
  // events that are called before others to be handled firts because they had
  // maller lag
  connection_params parameters;

  /**
    Next 4 are used for read event. When read() is called, passed parameters
    go into these queues.
  **/
  std::queue<char*> buffers;
  std::queue<unsigned int> buffers_sizes;
  std::queue<unsigned int> expect_to_read;
  std::queue<int> read_ids;
  /**
    Bytes that are already read, but not yet passed to the handler. When
    bytes_read is equal to expect_to_read, on_message_received is called and
    bytes_read becomes 0.
  **/
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
  /**
    Clears queues related to read operation. It is used when disconnecting.
    Could be removed if connections are unique and won't be reused.
  **/
  void clear_queues();
};

class simulated_acceptor: public acceptor {
 public:
  std::string address;
  bool started_accepting;
  connection::connection_handler* accepted_connections_handler;
  simulated_acceptor(const std::string& address_, acceptor::acceptor_handler*
      handler_, connection::connection_handler* accepted_connections_handler);
  void start_accepting();
  bool parse_address(const std::string& address);  // not implemented yet
  acceptor_handler* get_handler();
};

void logging(const std::string& s);

#endif  // NETWORK_SIMULATED_CONNECTION_H__
