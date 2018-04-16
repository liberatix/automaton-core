#include "network/tcp_implementation.h"
#include "data/protobuf_schema.h"
#include "node/node.h"

const char* address1 = "127.0.0.1:12311";
const char* address2 = "127.0.0.1:12322";
const char* address3 = "127.0.0.1:12333";
const char* address4 = "127.0.0.1:12344";
const char* address5 = "127.0.0.1:12355";
const char* address6 = "127.0.0.1:12366";
const char* address7 = "127.0.0.1:12377";

/////// PROTOCOL B ///////////////
/////// PROTOCOL C ///////////////
//////////////////////////////////

/////// NODE PROTOCOL (Protocol A) ///////////////

const char* node_protocol_def =
"syntax = \"proto3\";"
""
"message node_protocol {"
"  enum type {"
"    invalid = 0;"
"    hello = 1;"
"    ping = 2;"
"    pong = 3;"
"    need_peers = 4;"
"    sending_peers = 5;"
"    random_message = 6;"
"  }"
"  type message_type = 1;"
"  string data = 2;"
"}";

class node_protocol: public protocol {
  protobuf_schema schema;
  node* node_;

 public:
  node_protocol(node* node__): protocol("A"),  // NOLINT
      node_(node__) {
    schema.import_schema_from_string(node_protocol_def, "main", "");
  }
  ~node_protocol() {}
  int validate_message(const std::string& message) {
    int id = schema.new_message(0);
    schema.deserialize_message(id, message);
    if (schema.get_enum(id, 1) > 1 && schema.get_enum(id, 1) < 7) {
      return id;
    } else if (schema.get_enum(id, 1) == 1) {
      if (schema.get_string(id, 2) != "") {
        return id;
      }
    }
    return -1;
  }
  void process_message(const std::string& message, peer* peer_) {
    int id = validate_message(message);
    if (id == -1) {
      logging("Invalid message given to process: " + string_to_hex(message));
      return;
    }
    tcp_connection* connection_ =
        reinterpret_cast<tcp_connection*>(peer_->peer_connection);
    int message_type = schema.get_enum(id, 1);
    switch (message_type) {
      case 1: {
        logging(thread_id() + " --> " + "In Node protocol handler -> "
            "Hello FROM peer " + connection_->get_address() + " peer id is: " +
            schema.get_string(id, 2));
        break;
      }
      case 2: {
        logging(thread_id() + " --> " + "In Node protocol handler -> "
            "Ping FROM peer " + connection_->get_address());
            // send pong
        break;
      }
      case 3: {
        logging(thread_id() + " --> " + "In Node protocol handler -> "
            "Pong FROM peer " + connection_->get_address());
            // TODO(kari): change state to can connect
        break;
      }
      case 4: {
        logging(thread_id() + " --> " + "In Node protocol handler -> "
            "'get peer list' FROM peer " + connection_->get_address());
        break;
      }
      case 5: {
        logging(thread_id() + " --> " + "In Node protocol handler -> "
            "peer list received FROM peer " + (connection_->get_address()));
        break;
      }
      case 6: {
        logging(thread_id() + " --> " + "In Node protocol handler -> "
            "message FROM peer " + (connection_->get_address()) + ": " +
            schema.get_string(id, 2));
        break;
      }
      default: {
        logging(thread_id() + " --> " + "In Node protocol handler -> "
            "Unknown message type FROM peer " + (connection_->get_address()) +
            ": " + schema.get_string(id, 2));
        return;
      }
    }
  }

  void send_message(peer* peer_, int message_type, const std::string& message) {
    if (message_type < 1 && message_type > 6) {
      logging("Trying to send invalid message! Sending aborted!");
      return;
    }
    std::string new_message;
    int id = schema.new_message(0);
    schema.set_enum(id, 1, message_type);
    tcp_connection* connection_ =
        reinterpret_cast<tcp_connection*>(peer_->peer_connection);
    switch (message_type) {
      case 1: {
        logging(thread_id() + " --> " + "In Node protocol handler -> "
            "Hello TO peer " + connection_->get_address());
        schema.set_string(id, 2, node_->get_id());
        break;
      }
      case 2: {
        logging(thread_id() + " --> " + "In Node protocol handler -> "
            "Ping TO peer " + connection_->get_address());
        break;
      }
      case 3: {
        logging(thread_id() + " --> " + "In Node protocol handler -> "
            "Pong TO peer " + connection_->get_address());
        break;
      }
      case 4: {
        logging(thread_id() + " --> " + "In Node protocol handler -> "
            "'get peer list' TO peer " + connection_->get_address());
        break;
      }
      case 5: {
        logging(thread_id() + " --> " + "In Node protocol handler -> "
            "peer list send TO peer " + (connection_->get_address()));
        schema.set_string(id, 2, "peer_list");
        break;
      }
      case 6: {
        logging(thread_id() + " --> " + "In Node protocol handler -> "
            "message send TO peer " + (connection_->get_address()) + ": "
            + string_to_hex(std::string(message)));
            schema.set_string(id, 2, message);
        break;
      }
      default: {
        logging(thread_id() + " --> " + "In Node protocol handler -> "
            "Try to send unknown message type TO peer " +
            (connection_-> get_address()) + ": " + string_to_hex(message));
        return;
      }
    }
    schema.serialize_message(id, &new_message);
    peer_->send_message(node_->add_header(new_message, "A"));
  }
};

const char* second_protocol_def =
"syntax = \"proto3\";"
""
"message node_protocol {"
"  enum type {"
"    invalid = 0;"
"    poke = 1;"
"    random_message = 2;"
"  }"
"  type message_type = 1;"
"  string data = 2;"
"}";

class second_protocol: public protocol {
  protobuf_schema schema;
  node* node_;

 public:
  second_protocol(node* node__): protocol("B"),  // NOLINT
      node_(node__) {
    schema.import_schema_from_string(second_protocol_def, "main", "");
  }
  ~second_protocol() {}
  int validate_message(const std::string& message) {
    int id = schema.new_message(0);
    schema.deserialize_message(id, message);
    if (schema.get_enum(id, 1) == 1 || schema.get_enum(id, 1) == 2) {
      return id;
    }
    return -1;
  }
  void process_message(const std::string& message, peer* peer_) {
    int id = validate_message(message);
    if (id == -1) {
      logging("Invalid message given to process: " + string_to_hex(message));
      return;
    }
    tcp_connection* connection_ =
        reinterpret_cast<tcp_connection*>(peer_->peer_connection);
    int message_type = schema.get_enum(id, 1);
    switch (message_type) {
      case 1: {
        logging(thread_id() + " --> " + "In Second protocol handler -> "
            "Poke FROM peer " + connection_->get_address());
        break;
      }
      case 2: {
        logging(thread_id() + " --> " + "In Second protocol handler -> "
            "Message FROM peer " + (connection_->get_address()) + ": " +
            schema.get_string(id, 2));
        break;
      }
      default: {
        logging(thread_id() + " --> " + "In Second protocol handler -> "
            "Unknown message type FROM peer " + (connection_->get_address()) +
            ": " + schema.get_string(id, 2));
        return;
      }
    }
  }

  void send_message(peer* peer_, int message_type, const std::string& message) {
    if (message_type != 1 && message_type != 2) {
      logging("Trying to send invalid message! Sending aborted!");
      return;
    }
    std::string new_message;
    int id = schema.new_message(0);
    schema.set_enum(id, 1, message_type);
    tcp_connection* connection_ =
        reinterpret_cast<tcp_connection*>(peer_->peer_connection);
    switch (message_type) {
      case 1: {
        logging(thread_id() + " --> " + "In Second protocol handler -> "
            "Poke TO peer " + connection_->get_address());
        break;
      }
      case 2: {
        logging(thread_id() + " --> " + "In Second protocol handler -> "
            "Message send TO peer " + (connection_->get_address()) + ": "
            + string_to_hex(std::string(message)));
            schema.set_string(id, 2, message);
        break;
      }
      default: {
        logging(thread_id() + " --> " + "In Second protocol handler -> "
            "Try to send unknown message type TO peer " +
            (connection_-> get_address()) + ": " + string_to_hex(message));
        return;
      }
    }
    schema.serialize_message(id, &new_message);
    peer_->send_message(node_->add_header(new_message, "B"));
  }
};

/////// TEST ////////////////

void thread1() {
  try {
    node new_node("thisisotherme", {});
    node_protocol* node_proto = new node_protocol(&new_node);
    second_protocol* second_proto = new second_protocol(&new_node);
    new_node.add_protocol(node_proto);
    new_node.init();
    acceptor* acceptor2 = acceptor::create("tcp", address2,
        new_node.acceptor_handler_, new_node.connection_handler_);
    new_node.add_acceptor(acceptor2);
    acceptor2->start_accepting();

    peer* peer1 = new peer("first_peer_id", address1, 256,
        new_node.connection_handler_);
    new_node.add_peer(peer1);
    peer1->connect();
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    node_proto->send_message(peer1, 2, "This string does NOT matter for this"
        " message type");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    node_proto->send_message(peer1, 1, "This string does NOT matter for this"
        " message type");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    node_proto->send_message(peer1, 6, "This string DOES matter for this "
        "message type");
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    peer1->disconnect();

    peer1->connect();
    peer1->async_read(2);
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    node_proto->send_message(peer1, 6, "This is send after disconnect and"
        " connect again");
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    node_proto->send_message(peer1, 2, "");
    second_proto->send_message(peer1, 1, "");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    node_proto->send_message(peer1, 1, "");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    second_proto->send_message(peer1, 2, "ALABALA");
    node_proto->send_message(peer1, 6, "");
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    peer1->disconnect();
  } catch(std::exception& e) {
    logging("Exception: " + std::string(e.what()) + " IN THREAD " +
        thread_id());
  } catch (...) {
    logging("UNKNOWN ERROR IN THREAD " + thread_id());
  }
}

int main(int argc, char* argv[]) {
  try {
    node new_node("thisisme", {});
    new_node.add_protocol(new node_protocol(&new_node));
    new_node.add_protocol(new second_protocol(&new_node));
    new_node.init();
    acceptor* acceptor1 = acceptor::create("tcp", address1,
        new_node.acceptor_handler_, new_node.connection_handler_);
    new_node.add_acceptor(acceptor1);
    acceptor1->start_accepting();
    std::thread thread_(thread1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    thread_.join();
  } catch(std::exception& e) {
    logging("Exception: " + std::string(e.what()) + " IN THREAD " +
        thread_id());
    return 1;
  } catch (...) {
    logging("UNKNOWN ERROR IN THREAD " + thread_id());
    return 1;
  }
  return 0;
}
