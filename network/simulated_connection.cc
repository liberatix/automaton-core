#include "network/simulated_connection.h"

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <utility>

/**
  These will be removed when acceptor's address could be parsed. For now are
  used when acceptor creates and pass new connection.
**/
const int DEFAULT_MAX_CONNECTIONS = 8;
const int DEFAULT_BANDWITH = 64;

bool simulation::q_comparator::operator() (const event& lhs, const event& rhs)
    const {
  if (lhs.time_of_handling == rhs.time_of_handling) {
    return lhs.event_id > rhs.event_id;
  }
  return lhs.time_of_handling > rhs.time_of_handling;
}

unsigned int event::event_ids = 0;
event::event() {
  event_id = new_id();
  type = event_type::default_event;
  time_of_handling = 0;
  connection_ = nullptr;
  acceptor_ = nullptr;
  data = "";
  id = 0;
}
unsigned int event::new_id() {
  return ++event_ids;
}
std::string event::to_string() const {
  std::stringstream output;
  switch (type) {
    case 0:
      output << "event: none / ";
      break;
    case 1:
      output << "event: disconnect / ";
      break;
    case 2:
      output << "event: connection_attempt / ";
      break;
    case 3:
      output << "event: send / ";
      break;
    case 4:
      output << "event: read / ";
      break;
    case 5:
      output << "event: accept / ";
      break;
    case 6:
      output << "event: refuse / ";
      break;
    case 7:
      output << "event: error / ";
      break;
  }
  output << "time:" << time_of_handling << " / ";
  if (connection_) {
    output << "connection:" << connection_->get_address() << " / ";
  }
  if (acceptor_) {
    output << "acceptor:" << acceptor_->address << " / ";
  }
  output << "data:" << data << " / id:" << id;
  return output.str();
}

// SIMULATION

simulation* simulation::simulator = NULL;

simulation::simulation():simulation_time(0) {
  connection::register_connection_type("sim", [](const std::string& address,
      connection::connection_handler* handler) {
    return reinterpret_cast<connection*>(
        new simulated_connection(address, handler));
  });
  acceptor::register_acceptor_type("sim", [](const std::string& address,
      acceptor::acceptor_handler* handler_, connection::connection_handler*
      connections_handler_) {
    return reinterpret_cast<acceptor*>(new
        simulated_acceptor(address, handler_, connections_handler_));
  });
}
simulation::~simulation() {}

void simulation::push_event(const event& event_) {
  std::lock_guard<std::mutex> lock(q_mutex);
  event_q.push(event_);
}
simulation* simulation::get_simulator() {
  if (simulator) {
    return simulator;
  }
  return simulator = new simulation();
}
void simulation::handle_event(const event& e) {
  //  logging("Handling " + e.to_string());
    simulation* sim = simulation::get_simulator();
    switch (e.type) {
      case event_type::disconnect: {
        /**
          This event is created when the other endpoint has called disconnect().
          Sets connection state to disconnected.
          TODO(kari): If possible delete related events in the queue and
          delete connection fron map.
        **/
        if (e.connection_->connection_state == connection::state::connected) {
          e.connection_->connection_state = connection::state::disconnected;
          e.connection_->get_handler()->on_disconnected(e.connection_);
        } else if (e.connection_->connection_state ==
            connection::state::connecting) {
            logging("error? : disconnecting but not connected yet");
          // error?
          e.connection_->connection_state = connection::state::disconnected;
        }
        break;
      }
      case event_type::connection_attempt: {
        /**
          This event is created when the other endpoint has called connect().
          On_requested is called and if it returns true, new event with type
          accept is created, new connection is created from this endpoint to
          the other, the new connection's state is set to connected and
          on_connect in acceptor's handler is called. If the connections is
          refused, new event type refuse is created.
        **/
        event new_event;
        simulated_acceptor* acceptor_ = nullptr;
        if (!e.acceptor_) {
          acceptor_ = sim->get_acceptor(e.connection_->address_to);
          if (!acceptor_ || !(acceptor_->started_accepting)) {
            logging("ERROR: No such peer: " + e.connection_->address_to);
            // TODO(kari): error no such peer /
            break;
          }
        } else {
          acceptor_ = e.acceptor_;
        }
        if (acceptor_->get_handler()->on_requested(
            std::to_string(e.connection_->address_from))) {
          // logging("accepted");
          new_event.type = event_type::accept;
          new_event.connection_ = e.connection_;
          simulated_connection* new_connection =
              sim->get_connection(e.connection_->address_to,
              e.connection_->address_from);
          if (!new_connection) {
            std::string new_addr =
                std::to_string(e.connection_->parameters.min_lag) + ":" +
                std::to_string(e.connection_->parameters.max_lag) + ":" +
                std::to_string(DEFAULT_MAX_CONNECTIONS) + ":" +
                std::to_string(DEFAULT_BANDWITH) + ":" +
                std::to_string(e.connection_->address_to) + ":" +
                std::to_string(e.connection_->address_from);
            new_connection = new simulated_connection(new_addr,
                acceptor_->accepted_connections_handler);
            sim->add_connection(e.connection_->address_to,
                e.connection_->address_from, new_connection);
          }
          new_connection->connection_state = connection::state::connected;
          acceptor_->get_handler()->on_connected(new_connection,
              std::to_string(e.connection_->address_from));
          // TODO(kari): ? Call or not the connection's handler on_connect() ??
        } else {
          // logging("refused");
          new_event.type = event_type::refuse;
          new_event.connection_ = e.connection_;
        }
        new_event.event_id = event::new_id();
        connection_params* params = &(new_event.connection_->parameters);
        new_event.time_of_handling = (sim->get_time() >
            new_event.connection_->last_event ? sim->get_time() :
            new_event.connection_->last_event) + std::rand()%(params->max_lag -
            params->min_lag) + params->min_lag;
        new_event.connection_->last_event = new_event.time_of_handling;
        sim->push_event(new_event);
        break;
      }
      case event_type::send: {
        /**
          This event is created when send() is called. When tha lag passes it
          means that the other endpoint has received the message and read event
          for the other endpoint is created. The time of the newly created read
          event is the same as this send event. If the other endpoint exists
          and no error has happend, on_message_sent() is called.
          NOTE(kari):
          If at this point connection state is connecting (connect is called but
          accept/refuse is still not handled), this event is postponed.
          If the last case gives error or if in send() checks if state is
          connected, this means that process() should always be called after
          connect and before sending message which is not very comfortable when
          using the simulation. That's why the event is just postponed untill
          it is known if the connection is accepted or refused. BUG: If refused,
          error 'operation cancelled' will happend, which also happen when
          disconnect has been called.
        **/
        if (e.connection_->connection_state ==
            connection::state::disconnected) {
          logging("ERROR: Operation cancelled");  // Or connection refused
          break;
        }
        if (e.connection_->connection_state ==
            connection::state::connecting) {
          logging("ERROR: Not connected yet. Event is postponed.");
          event new_event = e;
          new_event.event_id = event::new_id();
          new_event.time_of_handling = sim->get_time() + 1 >
              e.connection_->last_event ? sim->get_time() + 1:
              e.connection_->last_event;
          new_event.connection_->last_event = new_event.time_of_handling;
          sim->push_event(new_event);
          break;
        }
        simulated_connection* new_connection = sim->
            get_connection(e.connection_->address_to,
            e.connection_->address_from);
        if (!new_connection) {
          logging("ERROR: no such peer in handle_event:send");
          break;
        }
        if (new_connection->connection_state ==
            connection::state::disconnected) {
          e.connection_->get_handler()->on_message_sent(e.connection_, e.id,
              connection::error::broken_pipe);
          break;
        }
        e.connection_->get_handler()->on_message_sent(e.connection_, e.id,
            connection::error::no_error);
        event new_event;
        new_event.type = event_type::read;
        new_event.connection_ = new_connection;
        new_event.data = e.data;
        new_event.event_id = event::new_id();
        new_event.time_of_handling =
          sim->get_time() > new_connection->last_event ?
          sim->get_time() : new_connection->last_event;
        new_connection->last_event = new_event.time_of_handling;
        push_event(new_event);
        break;
      }
      case event_type::read: {
        /**
          This event is created when send event from the other endpoint is
          handled. If no read() has been called, this event is postponed.
        **/
        if (e.connection_->connection_state ==
            connection::state::disconnected) {
          // BUG(kari): if a disconnect and connect again happen but a message
          // come with delay and is not cancelled because of disconnect,
          // the message will still arrive: FIX: find a way to delete events
          // (use other structure) or use connection ids
          logging("ERROR: Operation cancelled");
          break;
        }
        if (e.connection_->connection_state ==
            connection::state::connecting) {
          logging("ERROR: Not connected yet. This should never happen.");
          break;
        }
        if (e.connection_->buffers.empty()) {
          logging("no read buffers, event postponed");
          event new_event = e;
          new_event.event_id = event::new_id();
          ++new_event.time_of_handling;
          push_event(new_event);
          break;
        }
        /**
          If read() is called and there is a message to be read, there are a few
          possibilities.
          1. Read is called with specified number of bytes to be read
            Here expected bytes and message size matter. If the message is
            smaller than expected bytes, the message is copied into the buffer
            and the rest of the bytes will be read next time.
            No on_message_received is called untill all expected bytes are read.
          2. Read is called without specified number of bytes to be read which
            means to read the maximum possible. Here buffer and message size
            matter.

            Bandwidth is taken into account in both cases. If the message is
            bigger than the bandwidth of this connection, only a part of it is
            copied into the buffer and new read event with time = time + 1 is
            created for the rest.
        **/
        bool read_some = e.connection_->expect_to_read.front() == 0;
        unsigned int to_read = e.connection_->parameters.bandwidth <=
            e.data.size() ? e.connection_->parameters.bandwidth : e.data.size();
        if (!read_some) {
          to_read = to_read <= (e.connection_->expect_to_read.front()
            - e.connection_->bytes_read) ? to_read :
            e.connection_->expect_to_read.front() - e.connection_->bytes_read;
        } else {
          to_read = to_read <= (e.connection_->buffers_sizes.front()
              - e.connection_->bytes_read) ? to_read :
              e.connection_->buffers_sizes.front() - e.connection_->bytes_read;
        }
        std::memcpy(e.connection_->buffers.front() + e.connection_->bytes_read,
            e.data.data(), to_read);
        e.connection_->bytes_read += to_read;
        // if it's time to call on_message_received (required number of bytes
        // are read)
        if ((!read_some && e.connection_->expect_to_read.front() ==
            e.connection_->bytes_read) || read_some) {
          e.connection_->get_handler()->on_message_received(e.connection_,
            e.connection_->buffers.front(), e.connection_->bytes_read,
            e.connection_->read_ids.front());
          e.connection_->buffers.pop();
          e.connection_->buffers_sizes.pop();
          e.connection_->expect_to_read.pop();
          e.connection_->read_ids.pop();
          e.connection_->bytes_read = 0;
        }
        if (to_read < e.data.size()) {
          event new_event = e;
          new_event.event_id = event::new_id();
          new_event.data = new_event.data.substr(to_read);
          ++new_event.time_of_handling;
          push_event(new_event);
        }
        break;
      }
      case event_type::accept: {
        // TODO(kari): if no such connection, acceptor on error/ disconnected
        e.connection_->connection_state = connection::state::connected;
        e.connection_->get_handler()->on_connected(e.connection_);
        break;
      }
      case event_type::refuse: {
        // TODO(kari): if no such connection, acceptor on error/ disconnected
        e.connection_->connection_state = connection::state::disconnected;
        e.connection_->get_handler()->on_error(e.connection_,
            connection::error::connection_refused);
        break;
      }
      case event_type::error: {
        break;
      }
      default:
        break;
    }
  }

void simulation::process(uint64_t time_) {
  event e;
  q_mutex.lock();
  while (!event_q.empty() && event_q.top().time_of_handling < get_time()) {
    logging("ERROR: Event with time of handling before current time -> " +
        event_q.top().to_string());
    event_q.pop();
  }
  for (uint64_t current_time = event_q.top().time_of_handling;
      current_time <= time_ && !event_q.empty(); ++current_time) {
    set_time(current_time);
    while (!event_q.empty() && event_q.top().time_of_handling == current_time) {
      e = event_q.top();
      event_q.pop();
      q_mutex.unlock();
      handle_event(e);
      q_mutex.lock();
    }
  }
  q_mutex.unlock();
  set_time(time_);
}

uint64_t simulation::get_time() {
  std::lock_guard<std::mutex> lock(time_mutex);
  return simulation_time;
}
void simulation::set_time(uint64_t time_) {
  std::lock_guard<std::mutex> lock(time_mutex);
  if (time_ < simulation_time) {
    logging("ERROR: Trying to set time < current time");
  } else {
    simulation_time = time_;
  }
}

void simulation::add_connection(int address_a, int address_b,
    simulated_connection* connection_) {
  std::lock_guard<std::mutex> lock(connections_mutex);
  connections[std::make_pair(address_a, address_b)] = connection_;
}
simulated_connection* simulation::get_connection(int address_a, int address_b) {
  std::lock_guard<std::mutex> lock(connections_mutex);
  auto iterator_ = connections.find(std::make_pair(address_a, address_b));
  if (iterator_ == connections.end()) {
    return nullptr;
  }
  return iterator_->second;
}
void simulation::add_acceptor(int address, simulated_acceptor* acceptor_) {
  std::lock_guard<std::mutex> lock(acceptors_mutex);
  acceptors[address] = acceptor_;
}
simulated_acceptor* simulation::get_acceptor(int address) {
  std::lock_guard<std::mutex> lock(acceptors_mutex);
  auto iterator_ = acceptors.find(address);
  if (iterator_ == acceptors.end()) {
    return nullptr;
  }
  return iterator_->second;
}

void simulation::print_q() {
  std::lock_guard<std::mutex> lock(q_mutex);
  while (!event_q.empty()) {
    logging(event_q.top().to_string());
    event_q.pop();
  }
}

void simulation::print_connections() {
  std::lock_guard<std::mutex> lock(connections_mutex);
  for (auto it = connections.begin(); it != connections.end(); ++it) {
    logging(it->second->get_address());
  }
}

// CONNECTION

simulated_connection::simulated_connection(const std::string& address_,
    connection_handler* handler_):connection(handler_), last_event(0),
    bytes_read(0) {
  connection_state = connection::state::disconnected;
  if (!parse_address(address_) || address_to == address_from) {
    logging("ERROR: Connection creation failed : " + address_);
  /*} else if (get_connection(address_from, address_to)) {
    logging("Connection creation failed: connection already exists: "
        + address_);*/
  } else {
    // logging("Connection created: " + address_);
    simulation::get_simulator()->add_connection(address_from, address_to, this);
  }
}

void simulated_connection::async_send(const std::string& message, unsigned
    int id = 0) {
  /**
   BUG(kari): This is removed because if stay, you should always call process
   after connect. Fix: When handling event send, if not connected yet, postpone
   it.
  **/
  /*if (connection_state != connection::state::connected) {
    handler->on_message_sent(this, id, connection::error::not_connected);
    return;
  }*/
  if (message.size() < 1) {
    // logging("Send called but no message: id -> " + std::to_string(id));
    return;
  }
  // logging("Send called with message <" + message + "> with id: " +
  //    std::to_string(id));
  simulation* sim = simulation::get_simulator();
  event new_event;
  new_event.type = event_type::send;
  new_event.connection_ = this;
  new_event.event_id = event::new_id();
  new_event.data = message;  // TODO(kari): is this a copy
  new_event.id = id;
  new_event.time_of_handling = (sim->get_time() > last_event ?
      sim->get_time() : last_event) + std::rand()%(parameters.max_lag -
      parameters.min_lag) + parameters.min_lag;
  last_event = new_event.time_of_handling;
  sim->push_event(new_event);
}
void simulated_connection::async_read(char* buffer, unsigned int buffer_size,
    unsigned int num_bytes = 0, unsigned int id = 0) {
  /**
    This function does not create event. Actual reading will happen when the
    other endpoint sends a message and it arrives (send event is handled and
    read event is created).
  **/
  if (num_bytes > buffer_size) {
    logging("ERROR: Buffer size is smaller than needed! Reading aborted!");
    return;
  }
  // logging("Setting buffers in connection: " + get_address());
  buffers.push(buffer);
  buffers_sizes.push(buffer_size);
  expect_to_read.push(num_bytes);
  read_ids.push(id);
}
bool simulated_connection::parse_address(const std::string& address) {
  // TODO(kari): Make better regex. First one didn't work so this one was used
  // to save time on thinking about regexes
  std::regex rgx_sim("(\\d+):(\\d+):(\\d+):(\\d+):(\\d+):(\\d+)");
  std::smatch match;
  /*logging(address);
  if (std::regex_match(address.begin(), address.end(), match, rgx_sim)) {
    for (int i = 0; i < match.size(); ++i) {
      logging(match[i]);
    }
  }
  return false;*/
  if (std::regex_match(address.begin(), address.end(), match, rgx_sim) &&
      match.size() == 7) {  // == size of the fields in connection_params + 3
    parameters.min_lag = std::stoi(match[1]);
    parameters.max_lag = std::stoi(match[2]);
    parameters.max_connections = std::stoi(match[3]);
    parameters.bandwidth = std::stoi(match[4]);
    address_from = std::stoi(match[5]);
    address_to = std::stoi(match[6]);
    return true;
  } else {
    logging("ERROR: Could not resolve address and parameters in " + address);
    parameters.min_lag = 0;
    parameters.max_lag = 0;
    parameters.max_connections = 0;
    parameters.bandwidth = 0;
    address_from = 0;
    address_to = 0;
  }
  return false;
}

connection::state simulated_connection::get_state() const {
  return connection_state;
}
std::string simulated_connection::get_address() const {
  return std::to_string(address_from) + ":" + std::to_string(address_to);
}
void simulated_connection::connect() {
  /**
    This function creates connection_attempt event and changes the connection
    state to connectig.
  **/
  // logging("Connect called");
  connection_state = connection::state::connecting;
  simulation* sim = simulation::get_simulator();
  event new_event;
  new_event.type = event_type::connection_attempt;
  new_event.connection_ = this;
  new_event.event_id = event::new_id();
  new_event.acceptor_ = sim->get_acceptor(address_to);
  new_event.data = "";
  new_event.time_of_handling = (sim->get_time() > last_event ?
      sim->get_time() : last_event) + std::rand()%(parameters.max_lag -
      parameters.min_lag) + parameters.min_lag;
  last_event = new_event.time_of_handling;
  sim->push_event(new_event);
}
void simulated_connection::disconnect() {
    // logging("Disconnect called");
    /**
      This function creates disconnect event, changes the connection
      state to disconnected and calls handler's on_disconnected().
    **/
    connection_state = connection::state::disconnected;
    clear_queues();
    simulation* sim = simulation::get_simulator();
    simulated_connection* connection_ =
        sim->get_connection(address_to, address_from);
    if (connection_) {
      event new_event;
      new_event.type = event_type::disconnect;
      new_event.connection_ = connection_;
      new_event.event_id = event::new_id();
      new_event.data = "";
      new_event.time_of_handling = (sim->get_time() > last_event ?
          sim->get_time() : last_event) + std::rand()%(parameters.max_lag -
          parameters.min_lag) + parameters.min_lag;
      last_event = new_event.time_of_handling;
      sim->push_event(new_event);
    }
    handler->on_disconnected(this);
}

connection::connection_handler* simulated_connection::get_handler() {
  return handler;
}

void simulated_connection::clear_queues() {
  std::queue<char*> empty_buffers;
  std::swap(buffers, empty_buffers);
  std::queue<unsigned int> empty_buffers_sizes;
  std::swap(buffers_sizes, empty_buffers_sizes);
  std::queue<unsigned int> empty_expect_to_read;
  std::swap(expect_to_read, empty_expect_to_read);
  std::queue<int> empty_read_ids;
  std::swap(read_ids, empty_read_ids);
}
// ACCEPTOR

simulated_acceptor::simulated_acceptor(const std::string& address_,
    acceptor::acceptor_handler* handler_,
    connection::connection_handler* connections_handler):
    acceptor(handler_), address(address_),
    started_accepting(false),
    accepted_connections_handler(connections_handler) {
  simulation::get_simulator()->add_acceptor(std::stoi(address_), this);
}

void simulated_acceptor::start_accepting() {
  started_accepting = true;
}

acceptor::acceptor_handler* simulated_acceptor::get_handler() {
  return handler;
}

static std::mutex console_mutex;
void logging(const std::string& s) {
  std::lock_guard<std::mutex> lock(console_mutex);
  std::cout << s << std::endl;
}

std::string string_to_hex(const std::string& input) {
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();

    std::string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i) {
        const unsigned char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}
