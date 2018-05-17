#ifndef NETWORK_SIMULATED_CONNECTION_H__
#define NETWORK_SIMULATED_CONNECTION_H__

#include <map>
#include <mutex>
#include <string>
#include <queue>
#include <vector>
#include <unordered_map>
#include <utility>

#include "network/acceptor.h"
#include "network/connection.h"

class simulated_connection;
class simulated_acceptor;

// this could be protobuf message
// TODO(kari): Shrink it
struct event {
  enum type {
    undefined = 0,
    disconnect = 1,
    connection_attempt = 2,
    send = 3,
    accept = 4,
    refuse = 5,
    ack_received = 6,
    error = 7
  };
  type type_;
  /// uint64_t time_created;
  uint64_t time_of_handling;
  /// acceptor's address OR connection's ID in the vector
  unsigned int recipient;
  std::string data;
  unsigned int message_id;
  event();
  std::string to_string() const;
};

// this could be protobuf message
struct connection_params {
  unsigned int min_lag;
  unsigned int max_lag;
  connection_params();
};

struct acceptor_params {
  unsigned int max_connections;
  unsigned int bandwidth;
  acceptor_params();
};

/**
  Singleton class running the simulation. Stores created acceptors,
  connections, events and simulation time.
*/
class simulation {
 private:
  /**
   Comparator for the event queue -> events with lower time come first, if
   time is equal, event id is used and events created first are handled first.
  */
  struct q_comparator {
    bool operator() (const event& lhs, const event& rhs) const;
  };

  /**
    Vector storing created connections.
  */
  std::vector<simulated_connection*> connections;
  std::mutex connections_mutex;

  /**
    Map storing created acceptors where the key is the address.
  */
  std::map<uint32_t, simulated_acceptor*> acceptors;
  std::mutex acceptors_mutex;

  /**
    Priority queue storing the events that need to be handled. Lower time of
    handlig means higher priority. If equal, lower event id (created earlier)
    means higher priority.
  */
  // std::priority_queue<event, std::vector<event>, q_comparator> event_q;
  std::unordered_map<uint64_t, std::vector<event>> events;
  std::mutex q_mutex;

  /**
    Simulation time. On create is 0.
  */
  uint64_t simulation_time;
  std::mutex time_mutex;

  /** The simulation instance. */
  static simulation* simulator;

  // Constructor.
  simulation();

  /**
    Function that handles the events from the queue. It is called from
    process().
  */
  void handle_event(const event& event_);

  /**
    Update the simulation time.
  */
  void set_time(uint64_t time);

 public:
  ~simulation();

  /**
    Returns pointer to the simulation instance;
  */
  static simulation* get_simulator();

  /**
    Add event to the event queue. It is called from connection
    functions(connect, send, etc.) or from handle_event
  */
  void push_event(const event& event_);

  /** Returns current simulation time */
  uint64_t get_time();

  bool is_queue_empty();

  /**
    Process all events from simulation_time to the given time.
  */
  int process(uint64_t time);
  void add_connection(simulated_connection* connection_);
  void remove_connection(unsigned int connection_id);
  void add_acceptor(uint32_t address, simulated_acceptor* acceptor_);
  simulated_connection* get_connection(unsigned int connection_index);
  simulated_acceptor* get_acceptor(uint32_t address);
  simulated_acceptor* get_acceptor_by_connection(unsigned int connection_id);

  // DEBUG
  void print_q();
  void print_connections();
};

class simulated_connection: public connection {
 public:
  uint32_t remote_address;
  unsigned int local_connection_id;
  unsigned int remote_connection_id;
  state connection_state;

  /**
    This is used when setting event time to prevent events that are called before others to be
    handled first because they had smaller lag. It shows the last time when THIS endpoint send
    something to the other (message, acknowlede). When the other send acknowlede of some kind,
    this time_stamp is NOT taken into account and it is possible 2 events for this connection to be
    in the event queue at the same time.
  */
  unsigned int time_stamp;  // time_stamp
  connection_params parameters;

  /**
    Next 4 are used for read event. When read() is called, passed parameters
    go into these queues.
  */
  std::queue<char*> buffers;
  std::queue<unsigned int> buffers_sizes;
  std::queue<unsigned int> expect_to_read;
  std::queue<unsigned int> read_ids;

  /**
    Bytes that are already read, but not yet passed to the handler. When
    bytes_read is equal to expect_to_read, on_message_received is called and
    bytes_read becomes 0.
  */
  unsigned int bytes_read;

  /**
    Queue storing message_id and how many bytes from this message have left and should be
    sent
  */
  std::queue<std::pair<unsigned int, unsigned int> > sending;

  std::queue<std::string> receive_buffer;

  simulated_connection(const std::string& address_, connection_handler* handler_);

  bool parse_address(const std::string& address);

  void async_send(const std::string& message, unsigned int message_id);

  void async_read(char* buffer, unsigned int buffer_size, unsigned int num_bytes, unsigned int id);
  void handle_read();

  state get_state() const;
  std::string get_address() const;
  unsigned int get_lag() const;
  void connect();
  void disconnect();
  connection_handler* get_handler();

  /**
    Clears queues related to read operation. It is used when disconnecting.
    Could be removed if connections are unique and won't be reused.
  */
  void clear_queues();
};

class simulated_acceptor: public acceptor {
 public:
  uint32_t address;  // first 13 bits acceprors, second 19 - ports/connections
  acceptor_params parameters;
  bool started_accepting;
  connection::connection_handler* accepted_connections_handler;
  simulated_acceptor(const std::string& address_, acceptor::acceptor_handler*
      handler_, connection::connection_handler* accepted_connections_handler);
  void start_accepting();
  bool parse_address(const std::string& address);  // not implemented yet
  acceptor_handler* get_handler();
};

// TODO(kari): use proper logging.
void logging(const std::string& s);

#endif  // NETWORK_SIMULATED_CONNECTION_H__
