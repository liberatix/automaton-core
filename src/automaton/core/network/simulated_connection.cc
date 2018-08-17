#include "automaton/core/network/simulated_connection.h"

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <utility>

#include "automaton/core/io/io.h"

namespace automaton {
namespace core {
namespace network {

// TODO(kari): Make thread safe.

event::event() {
  type_ = event::type::undefined;
  time_of_handling = 0;
  source = 0;
  destination = 0;
  data = "";
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
    case event::type::connection_request:
      output << "event: connection_request / ";
      break;
    case event::type::message:
      output << "event: message / ";
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
  output << "time:" << time_of_handling << " / source:" << source << " / destination:" <<
      destination << " / data:" << data;
  return output.str();
}

connection_params::connection_params():min_lag(0), max_lag(0), bandwidth(0) {}
acceptor_params::acceptor_params():max_connections(0), bandwidth(0) {}

// SIMULATION

simulation* simulation::simulator = NULL;

simulation::simulation():simulation_time(0), simulation_running(false) {
  connection::register_connection_type("sim", [](connection_id id, const std::string& address,
      connection::connection_handler* handler) {
    return reinterpret_cast<connection*>(new simulated_connection(id, address, handler));
  });
  acceptor::register_acceptor_type("sim", [](const std::string& address, acceptor::acceptor_handler* handler_,
      connection::connection_handler* connections_handler_) {
    return reinterpret_cast<acceptor*>(new simulated_acceptor(address, handler_, connections_handler_));
  });
  /// Makes connection id = 0 invalid
  // connections.push_back(nullptr);
  std::srand(816405263);
}

simulation::~simulation() {
  LOG(DEBUG) << "Simulation destructor";
}

void simulation::simulation_start(uint64_t millisec_step) {
  running_mutex.lock();
  simulation_running = true;
  running_mutex.unlock();
  running_thread = std::thread([this, millisec_step](){
    uint64_t current = 0;
    while (true) {
      running_mutex.lock();
      if (simulation_running == false) {
          running_mutex.unlock();
          break;
      } else {
        running_mutex.unlock();
      }
      process(current);
      std::this_thread::sleep_for(std::chrono::milliseconds(millisec_step));
      current += millisec_step;
    }
  });
}

void simulation::simulation_stop() {
  running_mutex.lock();
  simulation_running = false;
  running_mutex.unlock();
  running_thread.join();
}

void simulation::push_event(const event& event_) {
  std::lock_guard<std::mutex> lock(q_mutex);
  events[event_.time_of_handling].push_back(event_);
}

simulation* simulation::get_simulator() {
  if (simulator) {
    return simulator;
  }
  return simulator = new simulation();
}

void simulation::handle_event(const event& e) {
  // LOG(DEBUG) << "HANDLING: " + e.to_string();
  simulation* sim = simulation::get_simulator();
  switch (e.type_) {
    case event::type::disconnect: {
      // LOG(DEBUG) << "disconnect 0";
      /**
        This event is created when the other endpoint has called disconnect().
        Sets connection state to disconnected.
        TODO(kari): If possible delete related events in the queue and
        delete connection fron map. NOTE: Maybe it is better not to delete events
        so proper errors could be passed (operation cancelled, broken_pipe, etc.)
      */
      simulated_connection* destination = sim->get_connection(e.destination);
      if (!destination) {  // state == disconnected should never happen
        LOG(INFO) << "Event disconnect but remote peer has already disconnected or does not exist";
        return;
      } else if (destination->get_state() == connection::state::connecting) {
        LOG(INFO) << "Event disconnect but peer is not connected yet! This situation is not "
            "handled right now!";
        return;
      }
      LOG(INFO) << "Other peer closed connection in: " << destination->get_address();
      destination->set_state(connection::state::disconnected);
      destination->get_handler()->on_disconnected(destination->get_id());
      // TODO(kari): Clear queues and call handlers with error, delete connection ids
      destination->remote_connection_id = 0;
      destination->cancel_operations();
      destination->clear_queues();
      sim->remove_connection(destination->local_connection_id);
      destination->local_connection_id = 0;
      // LOG(DEBUG) << "disconnect 1";
      break;
    }
    case event::type::connection_request: {
        // LOG(DEBUG) << "attempt 0";
      /**
        This event is created when the other endpoint has called connect(). On_requested is called
        and if it returns true, new event with type accept is created, new connection is created
        from this endpoint to the other, the new connection's state is set to connected and
        on_connect in acceptor's handler is called. If the connections is
        refused, new event type refuse is created.
      **/
      simulated_connection* source = sim->get_connection(e.source);
      simulated_acceptor* acceptor_ = sim->get_acceptor(e.destination);
      if (!source) {
        LOG(ERROR) << "Connection request from unexisting peer: " << e.source;
        break;
      }
      if (!acceptor_ || acceptor_->get_state() != acceptor::state::accepting) {
        LOG(ERROR) << "No such peer: " << e.destination;
        // TODO(kari): error no such peer /
        // new_event.type_ = event::type::error;
        break;
      }
      std::string source_address = source->get_address();
      event new_event;
      new_event.destination = e.source;
      new_event.time_of_handling = sim->get_time() + source->get_lag();
      const connection_params& params = source->parameters;
      connection_id cid;
      if (acceptor_->get_handler()->on_requested(acceptor_, source_address, &cid)) {
        // LOG(DEBUG) << "accepted";
        new_event.type_ = event::type::accept;
        /**
          Remote address of the other connection is 0 which means connect to that address is not
          possible.
        */
        std::string new_addr =
            std::to_string(params.min_lag) + ":" + std::to_string(params.max_lag) + ":" +
            std::to_string(acceptor_->parameters.bandwidth) + ":0";
        simulated_connection* new_connection =
            new simulated_connection(cid, new_addr, acceptor_->accepted_connections_handler);
        if (new_connection->init()) {
          sim->add_connection(new_connection);
          source->parameters.bandwidth = new_connection->parameters.bandwidth =
              source->parameters.bandwidth <= new_connection->parameters.bandwidth ?
              source->parameters.bandwidth : new_connection->parameters.bandwidth;
          source->remote_connection_id = new_connection->local_connection_id;
          new_connection->set_state(connection::state::connected);
          new_connection->remote_connection_id = source->local_connection_id;
          new_connection->set_time_stamp(new_event.time_of_handling);
          acceptor_->get_handler()->on_connected(acceptor_, new_connection, source_address);
          new_connection->get_handler()->on_connected(cid);
        } else {
          LOG(ERROR) << "Error while initializing connection";
        }
      } else {
        // LOG(DEBUG) << "refused";
        new_event.type_ = event::type::refuse;
      }
      sim->push_event(new_event);
      // LOG(DEBUG) << "attempt 1.2";
      break;
    }
    case event::type::message: {
      // LOG(DEBUG) << "message 0";
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
      simulated_connection* source = sim->get_connection(e.source);
      simulated_connection* destination = sim->get_connection(e.destination);
      if (!destination || destination->get_state() != connection::state::connected) {
        LOG(ERROR) << "ERROR in handling send! Peer has disconnected or does not exist!";
        // TODO(kari): broken pipe
        break;
      }
      destination->receive_buffer.push(e.data);
      destination->reading_q_mutex.lock();
      if (destination->reading.size()) {
        destination->reading_q_mutex.unlock();
        destination->handle_read();
      } else {
        destination->reading_q_mutex.unlock();
      }
      source->sending_q_mutex.lock();
      if (source && source->sending.front().message.size() == source->sending.front().bytes_send) {
        source->sending_q_mutex.unlock();
        event new_event;
        new_event.type_ = event::type::ack_received;
        new_event.data = std::to_string(connection::error::no_error);
        new_event.source = e.destination;
        new_event.destination = e.source;
        uint32_t t = sim->get_time();
        uint32_t ts = destination->get_time_stamp();
        new_event.time_of_handling = (t > ts + 1 ? t : ts + 1) + destination->get_lag();
        destination->set_time_stamp(new_event.time_of_handling);
        sim->push_event(new_event);
      } else if (source && source->get_state() == connection::state::connected) {
        source->sending_q_mutex.unlock();
        source->handle_send();
      } else {
        source->sending_q_mutex.unlock();
      }
      // LOG(DEBUG) << "message 1";
      break;
    }
    case event::type::accept: {
      // LOG(DEBUG) << "accept 0";
      simulated_connection* destination = sim->get_connection(e.destination);
      if (!destination) {
        // new event error
        // LOG(DEBUG) << "accept 2";
        break;
      }
      destination->set_state(connection::state::connected);
      destination->get_handler()->on_connected(destination->get_id());
      destination->handle_send();
      // LOG(DEBUG) << "accept 1";
      break;
    }
    case event::type::refuse: {
      simulated_connection* destination = sim->get_connection(e.destination);
      // LOG(DEBUG) << "refuse 0";
      if (!destination) {
        // new event error
        // LOG(DEBUG) << "refuse 2";
        break;
      }
      destination->set_state(connection::state::disconnected);
      destination->get_handler()->on_error(destination->get_id(), connection::error::connection_refused);
      destination->cancel_operations();
      destination->clear_queues();
      // LOG(DEBUG) << "refuse 1";
      break;
    }
    case event::type::ack_received: {
      // if (connection_->connection_state == connection::state::disconnected) {
      //   // TODO(kari): Possible: broken_pipe, operation_cancelled
      // }
      simulated_connection* destination = sim->get_connection(e.destination);
      destination->sending_q_mutex.lock();
      if (destination && destination->sending.size()) {
        uint32_t id = destination->sending.front().id;
        destination->sending.pop();
        destination->sending_q_mutex.unlock();
        destination->handle_send();
        destination->get_handler()->on_message_sent(destination->get_id(), id,
            static_cast<connection::error>(std::stoi(e.data)));
      } else {
        destination->sending_q_mutex.unlock();
      }
      break;
    }
    case event::type::error: {
      break;
    }
    default:
      break;
  }
  // LOG(DEBUG) << "automaton/core// HANDLING";
}

int simulation::process(uint64_t time_) {
  // // // LOG(DEBUG) << "process time: " << time_;
  int events_processed = 0;
  q_mutex.lock();

  auto current_time = get_time();
  // // // LOG(DEBUG) << "Cur time: " << current_time << " process time: " << time_;
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
    LOG(ERROR) << "Trying to set time < current time";
  } else {
    simulation_time = time_;
  }
}

bool simulation::is_queue_empty() {
  std::lock_guard<std::mutex> lock(q_mutex);
  return events.empty();
}

void simulation::add_connection(simulated_connection* connection_) {
  static uint32_t id = 0;
  // TODO(kari): use unordered map
  if (connection_) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    if (connection_->local_connection_id) {
      connections.erase(connection_->local_connection_id);
    }
    connections[++id] = connection_;
    connection_->local_connection_id = id;
  }
}
void simulation::remove_connection(uint32_t connection_id) {
  std::lock_guard<std::mutex> lock(connections_mutex);
  auto iterator_ = connections.find(connection_id);
  if (iterator_ != connections.end()) {
    delete iterator_->second;
    connections.erase(iterator_);
  }
}
simulated_connection* simulation::get_connection(uint32_t id) {
  std::lock_guard<std::mutex> lock(connections_mutex);
  auto iterator_ = connections.find(id);
  if (iterator_ == connections.end()) {
    return nullptr;
  }
  return iterator_->second;
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
void simulation::remove_acceptor(uint32_t address) {
  std::lock_guard<std::mutex> lock(acceptors_mutex);
  auto iterator_ = acceptors.find(address);
  if (iterator_ != acceptors.end()) {
    delete iterator_->second;
    acceptors.erase(iterator_);
  }
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
  // std::lock_guard<std::mutex> lock(connections_mutex);
  // for (uint32_t i = 0; i < connections.size(); ++i) {
  //   LOG(INFO) << connections[i]->get_address();
  // }
}

// CONNECTION

simulated_connection::incoming_packet::incoming_packet(): buffer(nullptr), buffer_size(0),
    expect_to_read(0), id(0), bytes_read(0) {}
simulated_connection::outgoing_packet::outgoing_packet(): bytes_send(0), id(0) {}

simulated_connection::simulated_connection(connection_id id, const std::string& address_, connection_handler* handler_):
    connection(id, handler_), remote_address(0), local_connection_id(0), remote_connection_id(0), time_stamp(0),
    original_address(address_), connection_state(connection::state::invalid_state) {
}

simulated_connection::~simulated_connection() {
  LOG(DEBUG) << "Connection destructor";
}

bool simulated_connection::init() {
  connection_state = connection::state::disconnected;
  if (!parse_address(original_address, &parameters, &remote_address)) {
    std::stringstream msg;
    LOG(ERROR) << "ERROR: Connection creation failed! Could not resolve address and parameters in: "
        << original_address;
    return false;
  }
  return true;
}

void simulated_connection::async_send(const std::string& message, uint32_t id = 0) {
  if (message.size() < 1) {
    LOG(ERROR) << "Send called but no message: id -> " << id;
    return;
  }
  if (get_state() == disconnected) {
    LOG(ERROR) << "Cannot send message! Call connect first!";
    return;
  }
  // LOG(DEBUG) << "<async_send>";
  // LOG << "Send called with message <" + message + ">");
  outgoing_packet packet;
  packet.message = message;
  packet.bytes_send = 0;
  packet.id = id;
  // LOG(DEBUG) << "pushing message <" << message << "> with id: " << id;
  sending_q_mutex.lock();
  sending.push(std::move(packet));
  sending_q_mutex.unlock();
  if (get_state() == connection::state::connected) {
    handle_send();
  }
  // LOG(DEBUG) << "</async_send>";
}

void simulated_connection::handle_send() {
  sending_q_mutex.lock();
  /// nothing to send or waiting for ACK
  if (sending.empty() || sending.front().message.size() == sending.front().bytes_send) {
    sending_q_mutex.unlock();
    return;
  }
  // LOG(DEBUG) << "<handle_send>";
  simulation* sim = simulation::get_simulator();
  outgoing_packet packet = sending.front();
  sending_q_mutex.unlock();
  event new_event;
  new_event.type_ = event::type::message;
  new_event.source = local_connection_id;
  new_event.destination = remote_connection_id;
  if (parameters.bandwidth >= (packet.message.size() - packet.bytes_send)) {
    if (!packet.bytes_send) {
      new_event.data = packet.message;
    } else {
      new_event.data = packet.message.substr(packet.bytes_send);
    }
  } else {
      new_event.data = packet.message.substr(packet.bytes_send, parameters.bandwidth);
  }
  packet.bytes_send += new_event.data.size();
  uint32_t t = sim->get_time();
  uint32_t ts = get_time_stamp();
  new_event.time_of_handling = (t > ts + 1 ? t : ts + 1) + get_lag();
  set_time_stamp(new_event.time_of_handling);
  sim->push_event(new_event);
  // LOG(DEBUG) << "</handle_send>";
}

void simulated_connection::handle_read() {
  // LOG(DEBUG) << "<handle_read>";
  reading_q_mutex.lock();
  recv_buf_mutex.lock();
  while (receive_buffer.size() && reading.size()) {
    incoming_packet& packet = reading.front();
    bool read_some = packet.expect_to_read == 0;
    uint32_t max_to_read = read_some ? packet.buffer_size : packet.expect_to_read;
    while (receive_buffer.size() && packet.bytes_read < max_to_read) {
      uint32_t left_to_read = max_to_read - packet.bytes_read;
      std::string message = receive_buffer.front();
      if (left_to_read >= message.size()) {
        std::memcpy(packet.buffer + packet.bytes_read, message.data(), message.size());
        packet.bytes_read += message.size();
        receive_buffer.pop();
      } else {
        std::memcpy(packet.buffer + packet.bytes_read, message.data(), left_to_read);
        packet.bytes_read += left_to_read;
        receive_buffer.front() = message.substr(left_to_read);
      }
    }
    if (read_some || packet.bytes_read == packet.expect_to_read) {
      // TODO(kari): use T top = std::move(q.front()); q.pop();
      incoming_packet packet = std::move(reading.front());
      reading.pop();
      recv_buf_mutex.unlock();
      reading_q_mutex.unlock();
      handler->on_message_received(this->id, packet.buffer, packet.bytes_read, packet.id);
      reading_q_mutex.lock();
      recv_buf_mutex.lock();
    }
  }
  recv_buf_mutex.unlock();
  reading_q_mutex.unlock();
  // LOG(DEBUG) << "</handle_read>";
}

void simulated_connection::async_read(char* buffer, uint32_t buffer_size,
    uint32_t num_bytes = 0, uint32_t id = 0) {
  // LOG(DEBUG) << "<async_read>";
  /**
    This function does not create event. Actual reading will happen when the
    other endpoint sends a message and it arrives (send event is handled and
    read event is created).
  */
  if (num_bytes > buffer_size) {
    LOG(ERROR) << "ERROR: Buffer size " << buffer_size << " is smaller than needed (" <<
        num_bytes << ")! Reading aborted!";
    return;
  }
  // logging("Setting buffers in connection: " + get_address());
  incoming_packet packet;
  packet.buffer = buffer;
  packet.buffer_size = buffer_size;
  packet.expect_to_read = num_bytes;
  packet.id = id;
  reading_q_mutex.lock();
  reading.push(std::move(packet));
  reading_q_mutex.unlock();
  recv_buf_mutex.lock();
  if (receive_buffer.size()) {
    recv_buf_mutex.unlock();
    handle_read();
  } else {
    recv_buf_mutex.unlock();
  }
  // LOG(DEBUG) << "</async_read>";
}

bool simulated_connection::parse_address(const std::string& address_, connection_params* params,
    uint32_t* parsed_remote_address) {
  std::regex rgx_sim("(\\d+):(\\d+):(\\d+):(\\d+)");
  std::smatch match;
  if (std::regex_match(address_.begin(), address_.end(), match, rgx_sim) &&
      std::stoul(match[1]) <= std::stoul(match[2]) &&
      match.size() == 5) {
    params->min_lag = std::stoul(match[1]);
    params->max_lag = std::stoul(match[2]);
    params->bandwidth = std::stoul(match[3]);
    *parsed_remote_address = std::stoul(match[4]);
    return true;
  }
  return false;
}

connection::state simulated_connection::get_state() const {
  std::lock_guard<std::mutex> lock(state_mutex);
  return connection_state;
}

void simulated_connection::set_state(connection::state new_state) {
  std::lock_guard<std::mutex> lock(state_mutex);
  connection_state = new_state;
}

std::string simulated_connection::get_address() const {
  return "conn_id:" + std::to_string(local_connection_id) +
      "/remote_conn_id:" + std::to_string(remote_connection_id) +
      "/accptr:" + std::to_string(remote_address);
}

uint32_t simulated_connection::get_lag() const {
  if (parameters.min_lag == parameters.max_lag) {
    return parameters.min_lag;
  }
  return std::rand() % (parameters.max_lag - parameters.min_lag) + parameters.min_lag;
}

void simulated_connection::connect() {
  if (!remote_address) {
    LOG(ERROR) << "Cannot connect: No address to connect to!";
    return;
  }
  // LOG(DEBUG) << "<connect>";
  connection_state = connection::state::connecting;
  simulation* sim = simulation::get_simulator();
  sim->add_connection(this);
  event new_event;
  new_event.type_ = event::type::connection_request;
  new_event.source = local_connection_id;
  new_event.destination = remote_address;
  uint32_t t = sim->get_time();
  uint32_t ts = get_time_stamp();
  new_event.time_of_handling = (t > ts + 1 ? t : ts + 1) + get_lag();
  set_time_stamp(new_event.time_of_handling);
  sim->push_event(new_event);
  // LOG(DEBUG) << "</connect>";
}

void simulated_connection::disconnect() {
    // LOG(DEBUG) << "<disconnect>";
    if (connection_state != connection::state::connected) {
      return;
    }
    connection_state = connection::state::disconnected;
    simulation* sim = simulation::get_simulator();
    event new_event;
    new_event.type_ = event::type::disconnect;
    new_event.destination = remote_connection_id;
    uint32_t t = sim->get_time();
    uint32_t ts = get_time_stamp();
    new_event.time_of_handling = (t > ts + 1 ? t : ts + 1) + get_lag();
    set_time_stamp(new_event.time_of_handling);
    sim->push_event(new_event);
    remote_connection_id = 0;
    cancel_operations();
    clear_queues();
    sim->remove_connection(local_connection_id);
    local_connection_id = 0;
    handler->on_disconnected(this->id);
    // LOG(DEBUG) << "</disconnect>";
}

connection::connection_handler* simulated_connection::get_handler() {
  return handler;
}

void simulated_connection::set_time_stamp(uint32_t t) {
  std::lock_guard<std::mutex> lock(time_stamp_mutex);
  if (t > time_stamp) {
    time_stamp = t;
  }
}

uint32_t simulated_connection::get_time_stamp() const {
  std::lock_guard<std::mutex> lock(time_stamp_mutex);
  return time_stamp;
}

void simulated_connection::clear_queues() {
  std::lock_guard<std::mutex> sending_lock(sending_q_mutex);
  std::lock_guard<std::mutex> reading_lock(reading_q_mutex);
  std::lock_guard<std::mutex> recv_lock(recv_buf_mutex);
  std::queue<incoming_packet> empty_reading;
  std::swap(reading, empty_reading);
  std::queue<outgoing_packet> empty_sending;
  std::swap(sending, empty_sending);
  std::queue<std::string> empty_receive_buffer;
  std::swap(receive_buffer, empty_receive_buffer);
}

void simulated_connection::cancel_operations() {
  std::lock_guard<std::mutex> lock(sending_q_mutex);
  while (!sending.empty()) {
    handler->on_message_sent(this->id, sending.front().id, connection::error::closed_by_peer);
    sending.pop();
  }
  // TODO(kari): Check what error gives if read is cancelled
  // while (!read_ids.empty()) {    }
}

// ACCEPTOR

simulated_acceptor::simulated_acceptor(const std::string& address_, acceptor::acceptor_handler* handler_,
    connection::connection_handler* connections_handler):acceptor(handler_),
    accepted_connections_handler(connections_handler), address(0), original_address(address_),
    acceptor_state(acceptor::state::invalid_state) {
}

simulated_acceptor::~simulated_acceptor() {
  LOG(DEBUG) << "Acceptor destructor";
}

bool simulated_acceptor::init() {
  if (parse_address(original_address, &parameters, &address)) {
    if (!address) {
      LOG(ERROR) << "ERROR: Acceptor creation failed! Acceptor address should be > 0";
      return false;
    } else {
      simulation::get_simulator()->add_acceptor(address, this);
    }
  } else {
    LOG(ERROR) << "ERROR: Acceptor creation failed! Could not resolve address and parameters in: "
        << original_address;
    return false;
  }
  set_state(acceptor::state::not_accepting);
  return true;
}

bool simulated_acceptor::parse_address(const std::string& address_, acceptor_params* params, uint32_t* parsed_address) {
  std::regex rgx_sim("(\\d+):(\\d+):(\\d+)");
  std::smatch match;
  if (std::regex_match(address_.begin(), address_.end(), match, rgx_sim) && match.size() == 4) {
    params->max_connections = std::stoul(match[1]);
    params->bandwidth = std::stoul(match[2]);
    *parsed_address = std::stoul(match[3]);
    return true;
  }
  return false;
}

void simulated_acceptor::start_accepting() {
  if (!address) {
    LOG(ERROR) << "This should never happen! Acceptor's address is not valid! Could not accept";
  }
  set_state(acceptor::state::accepting);
}

acceptor::state simulated_acceptor::get_state() const {
  std::lock_guard<std::mutex> lock(state_mutex);
  return acceptor_state;
}

void simulated_acceptor::set_state(acceptor::state new_state) {
  std::lock_guard<std::mutex> lock(state_mutex);
  acceptor_state = new_state;
}

std::string simulated_acceptor::get_address() const {
  return original_address;
}

acceptor::acceptor_handler* simulated_acceptor::get_handler() {
  return handler;
}

}  // namespace network
}  // namespace core
}  // namespace automaton
