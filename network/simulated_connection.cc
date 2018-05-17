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
    case event::type::undefined:
      output << "event: none / ";
      break;
    case event::type::disconnect:
      output << "event: disconnect / ";
      break;
    case event::type::connection_attempt:
      output << "event: connection_attempt / ";
      break;
    case event::type::send:
      output << "event: send / ";
      break;
    case event::type::accept:
      output << "event: accept / ";
      break;
    case event::type::refuse:
      output << "event: refuse / ";
      break;
    case event::type::ack_received:
      output << "event: ack_received / ";
      break;
    case event::type::error:
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
/*
  if (events.count(event_.time_of_handling) == 0) {
    events.emplace(event_.time_of_handling, std::vector<event>());
  }
*/
  events[event_.time_of_handling].push_back(event_);
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
    throw std::runtime_error("ERROR: No such connection\n" + e.to_string());
    return;
  }
  switch (e.type_) {
    case event::type::disconnect: {
      // logging("disconnect 0");
      /**
        This event is created when the other endpoint has called disconnect().
        Sets connection state to disconnected.
        TODO(kari): If possible delete related events in the queue and
        delete connection fron map. NOTE: Maybe it is better not to delete events
        so proper errors could be passed (operation cancelled, broken_pipe, etc.)
      */
      if (connection_->connection_state == connection::state::connected) {
        logging("Other peer closed connection in: " + connection_->get_address());
        connection_->connection_state = connection::state::disconnected;
        connection_->get_handler()->on_disconnected(connection_);
        // TODO(kari): Clear queues and call handlers with error, delete connection ids
      }
      // logging("disconnect 1");
      break;
    }
    case event::type::connection_attempt: {
        // logging("attempt 0");
      /**
        This event is created when the other endpoint has called connect(). On_requested is called
        and if it returns true, new event with type accept is created, new connection is created
        from this endpoint to the other, the new connection's state is set to connected and
        on_connect in acceptor's handler is called. If the connections is
        refused, new event type refuse is created.
      **/
      if (connection_->connection_state == connection::state::disconnected) {
        logging("ERROR: Peer requesting connection has disconnected!");
        break;
      }
      event new_event;
      new_event.recipient = e.recipient;
      new_event.time_of_handling = sim->get_time() + connection_->get_lag();
      simulated_acceptor* acceptor_ = sim->get_acceptor(connection_->remote_address);
      if (!acceptor_ || !(acceptor_->started_accepting)) {
        logging("ERROR: Connection request but no such peer: " + connection_->get_address());
        // TODO(kari): error no such peer /
        // new_event.type_ = event::type::error;
        // logging("attempt 1.1");
        break;
      }
      connection_params* params = &(connection_->parameters);
      if (acceptor_->get_handler()->on_requested(connection_->get_address())) {
        // logging("accepted");
        new_event.type_ = event::type::accept;
        /**
          Remote address of the other connection is 0 which means connect to that address is not
          possible.
        */
        std::string new_addr =
            std::to_string(params->min_lag) + ":" + std::to_string(params->max_lag) + ":0";
        simulated_connection* new_connection =
            new simulated_connection(new_addr, acceptor_->accepted_connections_handler);
        connection_->remote_connection_id = new_connection->local_connection_id;
        new_connection->connection_state = connection::state::connected;
        new_connection->remote_connection_id = connection_->local_connection_id;
        new_connection->time_stamp = new_event.time_of_handling;
        acceptor_->get_handler()->on_connected(new_connection, connection_->get_address());
        new_connection->get_handler()->on_connected(new_connection);
      } else {
        // logging("refused");
        new_event.type_ = event::type::refuse;
      }
      sim->push_event(new_event);
      // logging("attempt 1.2");
      break;
    }
    case event::type::send: {
      // logging("send 0");
      /**
        This is when the message is received from connection in remote.
      */
      /**
      TODO(kari):
      1. if connection_->state == disconnected --> operation cancelled / closed by peer;
       remote connection won't be able to send ack to connection and needs to call on_disconnect
      2. if !remote_connection || remote_connection->get_state() != connection::state::connected
        --> broken_pipe (&& this is still connected; haven't received disconnect message yet)
      */
      simulated_connection* remote_connection =
          sim->get_connection(connection_->remote_connection_id);
      // logging("send 1");
      if (!remote_connection || remote_connection->get_state() != connection::state::connected) {
        logging("ERROR in handling send!");
        break;
      }
      if (e.message_id != connection_->sending.front().first) {
        throw std::invalid_argument("ERROR: Message handling order is incorrect! This should"
            "never happen!");
      }
      simulated_acceptor* acceptor_ = sim->get_acceptor_by_connection(e.recipient);
      if (!acceptor_) {
        throw std::runtime_error("ERROR: No acceptor acceptor found in both connections while"
            "handling read! This should never happen!");
        break;
      }
      // logging("send 5");
      unsigned int bandwidth = acceptor_->parameters.bandwidth;
      unsigned int bytes_left = connection_->sending.front().second;
      unsigned int bytes_to_send = bytes_left < bandwidth ? bytes_left : bandwidth;
      remote_connection->receive_buffer.push(e.data.substr((e.data.size() - bytes_left),
                                                            bytes_to_send));
      connection_->sending.front().second -= bytes_to_send;
      /// If the connection can read
      if (remote_connection->read_ids.size()) {
        remote_connection->handle_read();
      }
      if (connection_->sending.front().second == 0) {
        // logging("send 6");
        event new_event;
        new_event.type_ = event::type::ack_received;
        new_event.message_id = e.message_id;
        new_event.data = std::to_string(connection::error::no_error);
        new_event.recipient = connection_->local_connection_id;
        new_event.time_of_handling = (sim->get_time() > remote_connection->time_stamp + 1 ?
                                      sim->get_time() : remote_connection->time_stamp + 1)
                                      + connection_->get_lag();
        remote_connection->time_stamp = new_event.time_of_handling;
        sim->push_event(new_event);
        connection_->sending.pop();
      } else {
        // logging("send 7");
        // If the remote connection has not disconnected and still can send data
        if (connection_->get_state() == connection::state::connected) {
          event new_event = e;
          ++new_event.time_of_handling;
          sim->push_event(new_event);
        }
      }
      // logging("send 8");
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
    case event::type::ack_received: {
      // if (connection_->connection_state == connection::state::disconnected) {
      //   // TODO(kari): Possible: broken_pipe, operation_cancelled
      // }
      connection_->get_handler()->on_message_sent(connection_, e.message_id,
          static_cast<connection::error>(std::stoi(e.data)));
      break;
    }
    case event::type::error: {
      break;
    }
    default:
      break;
  }
}

int simulation::process(uint64_t time_) {
  std::cout << "process time: " << time_ << std::endl;
  int events_processed = 0;
  q_mutex.lock();

  auto current_time = get_time();
  std::cout << "Cur time: " << current_time << " process time: " << time_ << std::endl;
  while (current_time <= time_) {
    set_time(current_time);
    if (events.count(current_time)) {
      for (auto& e : events.at(current_time)) {
        events_processed++;
        q_mutex.unlock();
        handle_event(e);
        q_mutex.lock();
      }
      events.erase(current_time);
    }
    current_time++;
  }

  q_mutex.unlock();
  return events_processed;
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

bool simulation::is_queue_empty() {
  std::lock_guard<std::mutex> lock(q_mutex);
  return events.empty();
//  return event_q.empty();
}

void simulation::add_connection(simulated_connection* connection_) {
  if (connection_) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    connections.push_back(connection_);
    connection_->local_connection_id = connections.size() - 1;
  }
}
void simulation::remove_connection(unsigned int connection_id) {
  std::lock_guard<std::mutex> lock(connections_mutex);
  if (connection_id < connections.size()) {
  //  connections[connection_id] = nullptr;
  }
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
simulated_acceptor* simulation::get_acceptor_by_connection(unsigned int connection_id) {
  simulated_connection* connection_ = get_connection(connection_id);
  if (!connection_) {
    return nullptr;
  }
  simulated_acceptor* acceptor_ = get_acceptor(connection_->remote_address);
  if (!acceptor_) {
    simulated_connection* remote_connection = get_connection(connection_->remote_connection_id);
    if (!remote_connection) {
      return nullptr;
    }
    acceptor_ = get_acceptor(remote_connection->remote_address);
  }
  if (!acceptor_) {
    throw std::runtime_error("ERROR: No acceptor found! This should never happen!");
  }
  return acceptor_;
}

void simulation::print_q() {
  /*
  std::lock_guard<std::mutex> lock(q_mutex);
  while (!event_q.empty()) {
    logging(event_q.top().to_string());
    event_q.pop();
  }
  */
}

void simulation::print_connections() {
  std::lock_guard<std::mutex> lock(connections_mutex);
  for (unsigned int i = 0; i < connections.size(); ++i) {
    logging(connections[i]->get_address());
  }
}

// CONNECTION

simulated_connection::simulated_connection(const std::string& address_,
    connection_handler* handler_): connection(handler_), remote_address(0), local_connection_id(0),
        remote_connection_id(0), time_stamp(0), bytes_read(0) {
  connection_state = connection::state::disconnected;
  if (!parse_address(address_)) {
    throw std::runtime_error("ERROR: Connection creation failed! Could not resolve address and "
                            "parameters in: " + address_);
  }
  simulation::get_simulator()->add_connection(this);
  // logging("Connection created: " + address_);
}

void simulated_connection::async_send(const std::string& message, unsigned int id = 0) {
  if (message.size() < 1) {
    logging("Send called but no message: id -> " + std::to_string(id));
    return;
  }
  // logging("Send called with message <" + message + ">");
  // if (connection_state != connection::state::connected && ! remote_address)
  unsigned int bandwidth =
      simulation::get_simulator()->get_acceptor_by_connection(local_connection_id)->
      parameters.bandwidth;
  sending.push(std::make_pair(id, message.size()));
  simulation* sim = simulation::get_simulator();
  event new_event;
  new_event.type_ = event::type::send;
  new_event.recipient = local_connection_id;
  new_event.data = message;
  new_event.message_id = id;
  new_event.time_of_handling = (sim->get_time() > time_stamp + 1 ?
                                sim->get_time() : time_stamp + 1) + get_lag();
  /// Next message wont be sent in the middle of sending this
  time_stamp = new_event.time_of_handling + (message.size() / bandwidth) + 1;
  sim->push_event(new_event);
}
void simulated_connection::handle_read() {
  // logging("handle read");
  while (receive_buffer.size() && buffers.size()) {
    bool read_some = expect_to_read.front() == 0;
    unsigned int max_to_read =
        read_some ? buffers_sizes.front() : expect_to_read.front();
    while (receive_buffer.size() && bytes_read < max_to_read) {
      unsigned int left_to_read = max_to_read - bytes_read;
      std::string message = receive_buffer.front();
      if (left_to_read >= message.size()) {
        std::memcpy(buffers.front() + bytes_read, message.data(), message.size());
        bytes_read += message.size();
        receive_buffer.pop();
      } else {
        std::memcpy(buffers.front() + bytes_read, message.data(), left_to_read);
        bytes_read += left_to_read;
        receive_buffer.front() = message.substr(left_to_read);
      }
    }
    if (read_some || bytes_read == expect_to_read.front()) {
      char* buffer = buffers.front();
      unsigned int id = read_ids.front();
      unsigned int bytes = bytes_read;
      buffers.pop();
      buffers_sizes.pop();
      expect_to_read.pop();
      read_ids.pop();
      bytes_read = 0;
      handler->on_message_received(this, buffer, bytes, id);
    }
  }
}

void simulated_connection::async_read(char* buffer, unsigned int buffer_size,
    unsigned int num_bytes = 0, unsigned int id = 0) {
  // logging("async_read called");
  /**
    This function does not create event. Actual reading will happen when the
    other endpoint sends a message and it arrives (send event is handled and
    read event is created).
  */
  if (num_bytes > buffer_size) {
    logging("ERROR: Buffer size is smaller than needed! Reading aborted!");
    return;
  }
  // logging("Setting buffers in connection: " + get_address());
  buffers.push(buffer);
  buffers_sizes.push(buffer_size);
  expect_to_read.push(num_bytes);
  read_ids.push(id);
  if (receive_buffer.size()) {
    handle_read();
  }
}

bool simulated_connection::parse_address(const std::string& address) {
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
  }
  return false;
}

connection::state simulated_connection::get_state() const {
  return connection_state;
}

std::string simulated_connection::get_address() const {
  return "conn_id:" + std::to_string(local_connection_id) + "/remote_conn_id:" +
      std::to_string(remote_connection_id) + "/accptr:" + std::to_string(remote_address);
}

unsigned int simulated_connection::get_lag() const {
  if (parameters.min_lag == parameters.max_lag) {
    return parameters.min_lag;
  }
  return std::rand() % (parameters.max_lag - parameters.min_lag) + parameters.min_lag;
}

void simulated_connection::connect() {
  if (!remote_address) {
    throw std::runtime_error("Cannot connect: No address to connect to!");
  }
  // logging("Connect called");
  connection_state = connection::state::connecting;
  simulation* sim = simulation::get_simulator();
  event new_event;
  new_event.type_ = event::type::connection_attempt;
  new_event.recipient = local_connection_id;
  new_event.time_of_handling = (sim->get_time() > time_stamp + 1 ?
                                sim->get_time() : time_stamp + 1) + get_lag();
  time_stamp = new_event.time_of_handling;
  sim->push_event(new_event);
}

void simulated_connection::disconnect() {
    // logging("disconnected called");
    // if (connection_state != connection::state::connected) {
    //   return;
    // }
    // connection_state = connection::state::disconnected;
    // /**
    // TODO(kari): Call send and read handlers with error operation_cancelled /
    //   closed_by_peer and clear queues
    // */
    // simulation* sim = simulation::get_simulator();
    // simulated_connection* connection_ = sim->get_connection(remote_connection_id);
    // if (connection_) {
    //   event new_event;
    //   new_event.type_ = event::type::disconnect;
    //   new_event.recipient = remote_connection_id;
    //   new_event.time_of_handling = (sim->get_time() > time_stamp + 1 ?
    //                                 sim->get_time() : time_stamp + 1) + get_lag();
    //   time_stamp = new_event.time_of_handling;
    //   sim->push_event(new_event);
    // } else {
    //   // TODO(kari): What if the other endpoint is already disconnected?
    // }
    // handler->on_disconnected(this);
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
  std::queue<unsigned int> empty_read_ids;
  std::swap(read_ids, empty_read_ids);
  std::queue<std::pair<unsigned int, unsigned int> > empty_sending;
  std::swap(sending, empty_sending);
  std::queue<std::string> empty_receive_buffer;
  std::swap(receive_buffer, empty_receive_buffer);
}
// ACCEPTOR

simulated_acceptor::simulated_acceptor(const std::string& address_,
    acceptor::acceptor_handler* handler_,
    connection::connection_handler* connections_handler):
    acceptor(handler_), started_accepting(false),
    accepted_connections_handler(connections_handler) {
  if (parse_address(address_)) {
    if (!address) {
      throw std::runtime_error("ERROR: Acceptor creation failed! Acceptor address should be > 0");
    } else {
      simulation::get_simulator()->add_acceptor(address, this);
    }
  } else {
    throw std::runtime_error("ERROR: Acceptor creation failed! Could not resolve address and "
                            "parameters in " + address_);
    address = 0;
  }
}

bool simulated_acceptor::parse_address(const std::string& address_) {
  std::regex rgx_sim("(\\d+):(\\d+):(\\d+)");
  std::smatch match;
  if (std::regex_match(address_.begin(), address_.end(), match, rgx_sim) && match.size() == 4) {
    parameters.max_connections = std::stoul(match[1]);
    parameters.bandwidth = std::stoul(match[2]);
    address = std::stoul(match[3]);
    return true;
  }
  return false;
}

void simulated_acceptor::start_accepting() {
  if (!address) {
    logging("This should never happen! Acceptor's address is not valid! Could not accept");
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
