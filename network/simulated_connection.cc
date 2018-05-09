#include "network/simulated_connection.h"

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <utility>

event::event() {
  type_ = event::type::undefined;
  time_of_handling = 0;
  recipient = 0;
  data = "";
  message_id = 0;
}
std::string event::to_string() const {
  std::stringstream output;
  switch (type_) {
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
  if (recipient) {
    output << "connection id:" << recipient << " / ";
  }
  output << "data:" << data << " / id:" << message_id;
  return output.str();
}

connection_params::connection_params():min_lag(0), max_lag(0) {}
acceptor_params::acceptor_params():max_connections(0), bandwidth(0) {}

// SIMULATION

bool simulation::q_comparator::operator() (const event& lhs, const event& rhs) const {
  return lhs.time_of_handling > rhs.time_of_handling;
}

simulation* simulation::simulator = NULL;

simulation::simulation():simulation_time(0) {
  connection::register_connection_type("sim", [](const std::string& address,
      connection::connection_handler* handler) {
    return reinterpret_cast<connection*>(new simulated_connection(address, handler));
  });
  acceptor::register_acceptor_type("sim", [](const std::string& address,
      acceptor::acceptor_handler* handler_,
      connection::connection_handler* connections_handler_) {
    return reinterpret_cast<acceptor*>(new
        simulated_acceptor(address, handler_, connections_handler_));
  });
  /// Makes connection id = 0 invalid
  connections.push_back(nullptr);
  std::srand(816405263);
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
    // logging("HANDLING: " + e.to_string());
    simulation* sim = simulation::get_simulator();
    simulated_connection* connection_ = sim->get_connection(e.recipient);
    if (!connection_) {
      logging("ERROR: No such connection");
      return;
    }
    switch (e.type_) {
      case event::type::disconnect: {
        // logging("disconnect 0");
        /**
          This event is created when the other endpoint has called disconnect().
          Sets connection state to disconnected.
          TODO(kari): If possible delete related events in the queue and
          delete connection fron map.
        */
        if (connection_->connection_state == connection::state::connected) {
          connection_->connection_state = connection::state::disconnected;
          connection_->get_handler()->on_disconnected(connection_);
        } else if (connection_->connection_state ==
              connection::state::connecting) {
            logging("error? : disconnecting but not connected yet");
          // error?
          connection_->connection_state = connection::state::disconnected;
        }
        // logging("disconnect 1");
        break;
      }
      case event::type::connection_attempt: {
          // logging("attempt 0");
        /**
          This event is created when the other endpoint has called connect().
          On_requested is called and if it returns true, new event with type
          accept is created, new connection is created from this endpoint to
          the other, the new connection's state is set to connected and
          on_connect in acceptor's handler is called. If the connections is
          refused, new event type refuse is created.
        **/
        event new_event;
        simulated_acceptor* acceptor_ =
            sim->get_acceptor(connection_->remote_address);
        if (!acceptor_ || !(acceptor_->started_accepting)) {
          logging("ERROR: No such peer: " + connection_->remote_address);
          // TODO(kari): error no such peer /
          // logging("attempt 1.1");
          break;
        }
        connection_params* params = &(connection_->parameters);
        if (acceptor_->get_handler()->
            on_requested(connection_->get_address())) {
          // logging("accepted");
          new_event.type_ = event::type::accept;
          /**
            Remote address of the other connection is 0 which means connect to
            that address is not possible.
          **/
          std::string new_addr =
              std::to_string(params->min_lag) + ":" +
              std::to_string(params->max_lag) + ":0";
          simulated_connection* remote_connection = new simulated_connection(
              new_addr, acceptor_->accepted_connections_handler);
          connection_->remote_connection_id = remote_connection->local_connection_id;
          remote_connection->connection_state = connection::state::connected;
          remote_connection->remote_connection_id = connection_->local_connection_id;
          acceptor_->get_handler()->on_connected(remote_connection,
              connection_->get_address());
          // TODO(kari): ? Call or not the connection's handler on_connect() ??
        } else {
          // logging("refused");
          new_event.type_ = event::type::refuse;
        }
        new_event.recipient = e.recipient;
        new_event.time_of_handling = (sim->get_time() >
            connection_->time_stamp + 1 ? sim->get_time() :
            connection_->time_stamp + 1) + connection_->get_lag();
        connection_->time_stamp = new_event.time_of_handling;
        sim->push_event(new_event);
        // logging("attempt 1.2");
        break;
      }
      case event::type::send: {
        // logging("send 0");
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
        if (connection_->connection_state ==
            connection::state::disconnected) {
          logging("ERROR: Operation cancelled");  // Or connection refused
          /*
            connection_->get_handler()->on_message_sent(connection_, e.message_id,
              connection::error::operation_cancelled);
          */
          // logging("send 1.1");
          break;
        }
        if (connection_->connection_state ==
            connection::state::connecting) {
          logging("ERROR: Not connected yet. Event is postponed.");
          event new_event = e;
          new_event.time_of_handling = (sim->get_time() >
              connection_->time_stamp ? sim->get_time():
              connection_->time_stamp) + 1;
          connection_->time_stamp = new_event.time_of_handling;
          sim->push_event(new_event);
          // logging("send 1.2");
          break;
        }
        simulated_connection* remote_connection = sim->
            get_connection(connection_->remote_connection_id);
        if (!remote_connection) {
          logging("ERROR: no such peer in handle_event:send");
          // logging("send 1.3");
          break;
        }
        if (remote_connection->connection_state ==
            connection::state::disconnected) {
          connection_->get_handler()->on_message_sent(connection_, e.message_id,
              connection::error::broken_pipe);
          // logging("send 1.4");
          break;
        }
        connection_->get_handler()->on_message_sent(connection_, e.message_id,
            connection::error::no_error);
        event new_event;
        new_event.type_ = event::type::read;
        new_event.recipient = connection_->remote_connection_id;
        new_event.data = e.data;
        new_event.time_of_handling =
          sim->get_time() > remote_connection->time_stamp + 1?
          sim->get_time() : remote_connection->time_stamp + 1;
        remote_connection->time_stamp = new_event.time_of_handling;
        push_event(new_event);
        // logging("send 1.5");
        break;
      }
      case event::type::read: {
        // logging("read 0");
        /**
          This event is created when send event from the other endpoint is
          handled. If no read() has been called, this event is postponed.
        **/
        if (connection_->connection_state ==
            connection::state::disconnected) {
          // BUG(kari): if a disconnect and connect again happen but a message
          // come with delay and is not cancelled because of disconnect,
          // the message will still arrive: FIX: find a way to delete events
          // (use other structure) or use connection ids
          logging("ERROR: Operation cancelled");
          // logging("read 1.1");
          break;
        }
        if (connection_->connection_state ==
            connection::state::connecting) {
          logging("ERROR: Not connected yet. This should never happen.");
          // logging("read 1.2");
          break;
        }
        if (connection_->buffers.empty()) {
          // logging("no read buffers, event postponed");
          event new_event = e;
          ++new_event.time_of_handling;
          push_event(new_event);
          // logging("read 1.3");
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

        /**
          Getting the acceptor related to that connection so we can get the
          bandwidth.
        */
        simulated_acceptor* acceptor_ =
            sim->get_acceptor(connection_->remote_address);
        if (!acceptor_) {
          simulated_connection* remote_connection =
              sim->get_connection(connection_->remote_connection_id);
          if (!remote_connection) {
            logging("ERROR: No such peer");
            // broken_pipe?
            logging("read 1.5");
            return;
          }
          acceptor_ = sim->get_acceptor(remote_connection->remote_address);
        }
        bool read_some = connection_->expect_to_read.front() == 0;
        unsigned int bandwidth = acceptor_->parameters.bandwidth;
        unsigned int to_read =
            bandwidth <= e.data.size() ? bandwidth : e.data.size();
        if (!read_some) {
          to_read =
              to_read <= (connection_->expect_to_read.front()
              - connection_->bytes_read) ? to_read :
              connection_->expect_to_read.front() - connection_->bytes_read;
        } else {
          to_read = to_read <= (connection_->buffers_sizes.front()
              - connection_->bytes_read) ? to_read :
              connection_->buffers_sizes.front() - connection_->bytes_read;
        }
        std::memcpy(connection_->buffers.front() + connection_->bytes_read,
            e.data.data(), to_read);
        connection_->bytes_read += to_read;
        // if it's time to call on_message_received (required number of bytes
        // are read)
        if ((!read_some && connection_->expect_to_read.front() ==
            connection_->bytes_read) || read_some) {
          connection_->get_handler()->on_message_received(connection_,
            connection_->buffers.front(), connection_->bytes_read,
            connection_->read_ids.front());
          connection_->buffers.pop();
          connection_->buffers_sizes.pop();
          connection_->expect_to_read.pop();
          connection_->read_ids.pop();
          connection_->bytes_read = 0;
        }
        if (to_read < e.data.size()) {
          event new_event = e;
          new_event.data = new_event.data.substr(to_read);
          ++new_event.time_of_handling;
          push_event(new_event);
        } else {
          // TODO(kari): Call handler's on_message_sent here.
        }
        // logging("read 1.6");
        break;
      }
      case event::type::accept: {
        // logging("accept 0");
        // TODO(kari): if no such connection, acceptor on error/ disconnected
        connection_->connection_state = connection::state::connected;
        connection_->get_handler()->on_connected(connection_);
        // logging("accept 1");
        break;
      }
      case event::type::refuse: {
        // logging("refuse 0");
        // TODO(kari): if no such connection, acceptor on error/ disconnected
        connection_->connection_state = connection::state::disconnected;
        connection_->get_handler()->on_error(connection_,
            connection::error::connection_refused);
        // logging("refuse 1");
        break;
      }
      case event::type::error: {
        break;
      }
      default:
        break;
    }
  }

void simulation::process(uint64_t time_) {
  // TODO(kari): Exception -> deadlock
  event e;
  q_mutex.lock();
  while (!event_q.empty() && event_q.top().time_of_handling < get_time()) {
    logging("ERROR: Event with time of handling before current time -> " +
        event_q.top().to_string());
    event_q.pop();
  }
  for (uint64_t current_time = event_q.top().time_of_handling;
      current_time <= time_ && !event_q.empty(); ) {
    set_time(current_time);
    while (!event_q.empty() && event_q.top().time_of_handling == current_time) {
      e = event_q.top();
      event_q.pop();
      q_mutex.unlock();
      handle_event(e);
      q_mutex.lock();
    }
    if (!event_q.empty()) {
      current_time = event_q.top().time_of_handling;
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

void simulation::add_connection(simulated_connection* connection_) {
  std::lock_guard<std::mutex> lock(connections_mutex);
  connections.push_back(connection_);
  connection_->local_connection_id = connections.size() - 1;
}
simulated_connection* simulation::get_connection(uint32_t index) {
  std::lock_guard<std::mutex> lock(connections_mutex);
  if (index >= connections.size()) {
    return nullptr;
  }
  return connections[index];
}
void simulation::add_acceptor(uint32_t address, simulated_acceptor* acceptor_) {
  std::lock_guard<std::mutex> lock(acceptors_mutex);
  acceptors[address] = acceptor_;
}
simulated_acceptor* simulation::get_acceptor(uint32_t address) {
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
  for (unsigned int i = 0; i < connections.size(); ++i) {
    logging(connections[i]->get_address());
  }
}

// CONNECTION

simulated_connection::simulated_connection(const std::string& address_,
    connection_handler* handler_):connection(handler_), time_stamp(0),
    bytes_read(0) {
  connection_state = connection::state::disconnected;
  if (!parse_address(address_)) {
    logging("ERROR: Connection creation failed : " + address_);
    remote_address = 0;
  } else {
    // logging("Connection created: " + address_);
    simulation::get_simulator()->add_connection(this);
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
  // std::to_string(id));
  simulation* sim = simulation::get_simulator();
  event new_event;
  new_event.type_ = event::type::send;
  new_event.recipient = local_connection_id;
  new_event.data = message;  // TODO(kari): is this a copy
  new_event.message_id = id;
  new_event.time_of_handling =
      (sim->get_time() > time_stamp + 1 ? sim->get_time() : time_stamp + 1) + get_lag();
  time_stamp = new_event.time_of_handling;
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
  std::regex rgx_sim("(\\d+):(\\d+):(\\d+)");
  std::smatch match;
  /* logging(address);
  if (std::regex_match(address.begin(), address.end(), match, rgx_sim)) {
    for (int i = 0; i < match.size(); ++i) {
      logging(match[i]);
    }
  }
  return false; */
  if (std::regex_match(address.begin(), address.end(), match, rgx_sim) &&
      std::stoul(match[1]) <= std::stoul(match[2]) &&
      match.size() == 4) {  // == size of the fields in connection_params + 3
    parameters.min_lag = std::stoul(match[1]);
    parameters.max_lag = std::stoul(match[2]);
    remote_address = std::stoul(match[3]);
    return true;
  } else {
    logging("ERROR: Could not resolve address and parameters in " + address);
  }
  return false;
}

connection::state simulated_connection::get_state() const {
  return connection_state;
}
std::string simulated_connection::get_address() const {
  return "conn_id:" + std::to_string(local_connection_id) + "->accptr:" +
      std::to_string(remote_address);
}
unsigned int simulated_connection::get_lag() const {
  if (parameters.min_lag == parameters.max_lag) {
    return parameters.min_lag;
  }
  return std::rand()%(parameters.max_lag - parameters.min_lag) + parameters.min_lag;
}
void simulated_connection::connect() {
  if (!remote_address) {
    logging("Cannot connect: No address to connect to!");
  }
  /**
    This function creates connection_attempt event and changes the connection
    state to connectig.
  **/
  // logging("Connect called");
  connection_state = connection::state::connecting;
  simulation* sim = simulation::get_simulator();
  event new_event;
  new_event.type_ = event::type::connection_attempt;
  new_event.recipient = local_connection_id;
  new_event.data = "";
  new_event.time_of_handling =
      (sim->get_time() > time_stamp + 1 ? sim->get_time() : time_stamp + 1) + get_lag();
  time_stamp = new_event.time_of_handling;
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
        sim->get_connection(remote_connection_id);
    if (connection_) {
      event new_event;
      new_event.type_ = event::type::disconnect;
      new_event.recipient = local_connection_id;
      new_event.data = "";
      new_event.time_of_handling =
          (sim->get_time() > time_stamp + 1 ? sim->get_time() : time_stamp + 1)
          + get_lag();
      time_stamp = new_event.time_of_handling;
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
    acceptor(handler_), started_accepting(false),
    accepted_connections_handler(connections_handler) {
  if (parse_address(address_)) {
    if (!address) {
      logging("Error: Could not create acceptor. Acceptor address should be > 0");
    } else {
      simulation::get_simulator()->add_acceptor(address, this);
    }
  } else {
    logging("Error while creating acceptor");
    address = 0;
  }
}
bool simulated_acceptor::parse_address(const std::string& address_) {
  std::regex rgx_sim("(\\d+):(\\d+):(\\d+)");
  std::smatch match;
  if (std::regex_match(address_.begin(), address_.end(), match, rgx_sim) &&
      match.size() == 4) {  // == size of the fields in acceptor_params + 2
    parameters.max_connections = std::stoul(match[1]);
    parameters.bandwidth = std::stoul(match[2]);
    address = std::stoul(match[3]);
    return true;
  } else {
    logging("ERROR: Could not resolve address and parameters in " + address_);
  }
  return false;
}

void simulated_acceptor::start_accepting() {
  if (!address) {
    logging("Acceptor's address is not valid! Could not accept");
  }
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
