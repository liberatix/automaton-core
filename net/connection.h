#ifndef AUTOMATON_CORE_NET_CONNECTION_H__
#define AUTOMATON_CORE_NET_CONNECTION_H__

#include <string>
#include <map>

// TODO(kari): add state as a member and get_state
// TODO(kari): think about `send and disconnect`

/**
  Class that represents a connection between two peers. It is used to connect to
  remote peer or is used by the acceptor class when accepts an incoming 
  connection.
**/

class connection {
public:
  /**
    TODO(kari)
  **/
  enum state {
    invalid_state = 0,
    connecting = 1,
    connected = 2,
    disconnected = 3,
  };
  /**
    TODO(kari)
  **/
  enum error {
    unknown = 0,
    invalid_address = 1,
    closed_by_peer = 2,
    connection_refused = 3,
    timed_out = 4,
  };
  
  /** 
    Handler class used to inform the client for events.
      - on_message_received should be invoked when message from remote peer was 
        received; it passes the connection, representing from which peer was the
        message, and the message that was received.
      - on_message_sent should be invoked when a message was sent successfully 
        using async_send.
      TODO(kari): What happens if there is an error; how error pass the message 
        id.
      - on_connected should be invoked when the connection with the remote peer 
        was established.
      - on_disconnected should be invoked when connection was closed/ destroyed 
        by the local or the remote peer
      - on_error should be invoked when an error happens while listening or 
        accepting.
  **/
  class connection_handler {
  public:
    virtual void on_message_received(connection* c, const std::string& 
        message) = 0;
    virtual void on_message_sent(connection* c, int id) = 0;
    virtual void on_connected(connection* c) = 0;
    virtual void on_disconnected(connection* c) = 0;
    virtual void on_error(connection::error e) = 0;
  };
  
  /**
    Class constructor.
  **/
  connection(connection_handler* _handler);

  /**
    Function that is used to send message to the remote peer. Id shows the 
    sequence_id at the time of sending the message. On_message_sent should be 
    invoked once the message was sent successfully.
  **/
  virtual void async_send(const std::string& message, int id) = 0;

  /**
    Function that is used to create objects from a specified child class.
    The child class should first be registered using register_acceptor_type 
    function. The function returns object from the specified class. If no such 
    class type was registered, NULL will be returned.
  **/
  static connection* create(const std::string& type, const std::string& address,
      const std::string& port, connection_handler* handler);

  typedef connection* (*factory_function)(const std::string& address,
      const std::string& port, connection_handler* handler);
      
  /**
    Function that is used to register how an object from child class will be 
    created. Type shows how the class will be referenced (ex. "child_class_1"),
    factory_function specifies the function tht will be used to create an 
    object. See "typedef connection* (*factory_function)..." above to check what 
    arguments should this function accept. If such type name exists in the 
    registry, the factory_function pointer will be overriden.
  **/
  static void register_connection_type(const std::string& type, 
      factory_function func);
  
  /**
    Function that returns the number of the next message that will be sent. It 
    should be used ONLY to pass a message id when sending a message with 
    async_send and the result can be stored to keep track of unsuccessfully sent 
    messages that should be resent. It increments with one every time is 
    invoked.
    TODO(asen): TOTHINK: Is this really really needed?
  **/
  inline int get_next_sequence_id();

protected:
  /**
    Handler object that must be set so the client could be informed for events.
    If no handler or a handler with empty function implementations is provided, 
    client will not have access to events information like connect/ disconnect,
    received messages or an error that happend.
  **/
  
  connection_handler* handler;

private:
  /**
    Variable that represents size of a sequence. It can be used as ids of the
    messages that were sent. 
  **/
  int sequence_id;
  static std::map<std::string, factory_function> connection_factory;
};

#endif // AUTOMATON_CORE_NET_CONNECTION_H__

