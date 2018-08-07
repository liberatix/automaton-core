#ifndef AUTOMATON_CORE_SMARTPROTO_NODE_H_
#define AUTOMATON_CORE_SMARTPROTO_NODE_H_

#include <memory>
#include <string>
#include <vector>

#include "automaton/core/data/factory.h"
#include "automaton/core/data/msg.h"
#include "automaton/core/data/schema.h"
#include "automaton/core/script/lua/lua_script_engine.h"

namespace automaton {
namespace core {
namespace smartproto {

struct peer {};
struct peer_info {};

class node {
 public:
  node(std::unique_ptr<data::schema> schema,
       const std::string& lua_script);

 private:
  script::lua::lua_script_engine script_engine;
  std::vector<peer> connected_peers;
  std::vector<peer_info> known_peers;
  std::unique_ptr<data::factory> msg_factory;
  std::unique_ptr<data::schema> schema;
};

}  // namespace smartproto
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_SMARTPROTO_NODE_H_
