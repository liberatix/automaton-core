
global_seq = 1

function gossip(peer_id, msg)
  for k, v in pairs(peers) do
    if k ~= 0 and k ~= peer_id then
      global_seq = global_seq + 1
      log("sent", string.format("Sending to %s (%d)", peers[k].name, global_seq))
      mm = msg
      mm.global_sequence = global_seq
      send(k, mm, global_seq)
      if math.random(1000) < 20 then
        log("sent", "ATTEMPTING DISCONNECT BEFORE")
        disconnect(k)
        log("sent", "ATTEMPTING DISCONNECT AFTER")
      end
    end
  end
end

function sent(peer_id, msg_id, success)
  log("sent",
    string.format("Sent to %s (%d), success: %s", peers[peer_id].name, msg_id, tostring(success))
  )
end

msgs = {}
msg_index = 0

function on_Msg(peer_id, m)
  hash = hex(sha3(m.author .. m.msg))
  msg = string.format("<%s>: %s [FROM %s] [%s]", m.author, m.msg, peers[peer_id].name, hash)
  if msgs[hash] == nil then
    msgs[hash] = m.msg
    log("CHAT", msg)
    gossip(peer_id, m)
  else
    log("IGNORED", "[Already received] " .. msg)
  end
end

function update(timestamp)
  if math.random(100) < 5 then
    m = Msg()
    msg_index = msg_index + 1
    local idx = ((msg_index - 1) % #msg_contents) + 1

    m.sequence = msg_index
    m.author = nodeid;
    m.msg = msg_contents[idx] .. " (" .. tostring(msg_index) .. ")"
    on_Msg(0, m)
  end
end
