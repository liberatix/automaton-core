-- chat init.lua
history_add("testnet(localhost, chat_node, 5, 1, \"automaton/examples/smartproto/chat/\")")
-- testnet(localhost, chat_node, 5, 1, "automaton/examples/smartproto/chat/")
function chat_node(id)
  local n = node(id, "automaton/examples/smartproto/chat/")

  _G[id] = {
    node_type = "chat",

    disconnect_all = function()
      n:call("disconnect_all()")
    end,

    connect = function(peer_id)
      n:call("connect("..tostring(peer_id)..")")
    end,
  }

  return n
end
