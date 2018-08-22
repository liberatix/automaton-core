-- Keeps track of the currently connected peers

conn = {}
conn[0] = { name="ME!" }

function connected(peer_id)
  log("connections", "CONNECTED TO " .. tostring(peer_id))
  conn[peer_id] = { name = "N/A" }
  hi = Hello()
  hi.name = nodeid
  
  send(peer_id, hi, 1)
end

function disconnected(peer_id)
  log("connections", "DISCONNECTED FROM " .. tostring(peer_id))
  conn[peer_id] = nil
end

function on_Hello(peer_id, m)
  log("HELLO", "Hello from peer " .. tostring(peer_id) .. " name: " .. m.name)
  conn[peer_id].name = m.name
end
