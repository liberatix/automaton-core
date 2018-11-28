-- chat init.lua
history_add("testnet(localhost, chat_node, 5, 1, \"logs/chat/\")")
-- testnet(localhost, chat_node, 5, 1, "automaton/examples/smartproto/chat/")
function chat_node(id)
  local n = node(id, "automaton/examples/smartproto/chat/")

  _G[id] = {
    node_type = "chat",
  }

  return n
end
