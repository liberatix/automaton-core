-- chat.lue

math.randomseed(os.time())

global_seq = 1

function gossip(peer_id, msg)
  for k, v in pairs(peers) do
    if k ~= 0 and k ~= peer_id then
      global_seq = global_seq + 1
      mm = msg
      mm.global_sequence = global_seq
      send(k, mm, global_seq)
    end
  end
end

function sent(peer_id, msg_id, success)
  -- log("sent",
  --   string.format("Sent to %s (%d), success: %s", peers[peer_id].name, msg_id, tostring(success))
  -- )
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

wait = 100

function update(timestamp)
  wait = wait + 1
  if wait >= 30 then
    wait = 0
    m = Msg()
    msg_index = msg_index + 1
    local idx = ((msg_index - 1) % #msg_contents) + 1

    m.hash = sha3(nodeid .. tostring(msg_index))
    m.sequence = msg_index
    m.author = nodeid;
    m.msg = msg_contents[idx] .. " (" .. tostring(msg_index) .. ")"
    on_Msg(0, m)
  end
end
