-- connections.lua

-- Keeps track of the currently connected peers

conn = {}
conn[0] = { name="ME!" }

function pid(peer_id)
  if conn[peer_id] ~= nil then
    return string.format("CONN%d[%s]", peer_id, conn[peer_id].name)
  end
  return "N/A"
end

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
  peer_connected(peer_id)
  hash = cur_hash()
  last_block = get_block(hash)
  if last_block == nil then
    print("LAST BLOCK IS NIL! " .. hex(hash))
    print("blockchain: " .. #blockchain)
    return
  end
  conn[peer_id].last_hash = hash
  conn[peer_id].height = last_block.height
end
