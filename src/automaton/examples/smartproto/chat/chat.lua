peers = {}

function connected(peer_id)
  log("discovery", "CONNECTED TO " .. tostring(peer_id))
  peers[peer_id] = {}
end

function disconnected(peer_id)
  log("discovery", "DISCONNECTED FROM " .. tostring(peer_id))
  peers[peer_id] = nil
end

function gossip(peer_id, msg)
  for k, v in pairs(peers) do
    if k ~= peer_id then
      log("gossip", string.format("Sending to peer %d", k))
      send(k, msg)
    end
  end
end

msgs = {}

log("init", "Starting chat node")

function on_Msg(peer_id, m)
  log("on_Msg", string.format("Received message from %d", peer_id))
end

function sent(peer_id, msg_id, success)
  log("sent", "sent to " .. tostring(peer_id) .. " msgid: " .. msg_id .. " success: " .. tostring(success))
end

function update(timestamp)
  if math.random(100) < 10 then
    log("update", string.format("Initiating chat message ", timestamp))
    m = Msg()
    m.msg = "HELLO " .. tostring(timestamp)
    gossip(0, m)
  end
end
