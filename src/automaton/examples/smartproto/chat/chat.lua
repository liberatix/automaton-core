peers = {}
peers[0] = {name="ME!"}

function connected(peer_id)
  log("discovery", "CONNECTED TO " .. tostring(peer_id))
  peers[peer_id] = { name="N/A" }
  hi = Hello()
  hi.name = nodeid
  send(peer_id, hi, 1)
end

function disconnected(peer_id)
  log("discovery", "DISCONNECTED FROM " .. tostring(peer_id))
  peers[peer_id] = nil
end

global_seq = 1

function gossip(peer_id, msg)
  for k, v in pairs(peers) do
    if k ~= 0 and k ~= peer_id then
      global_seq = global_seq + 1
      log("sent", string.format("Sending to %s (%d)", peers[k].name, global_seq))
      send(k, msg, global_seq)
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
msg_contents = {
  "Hello",
  "How are you?",
  "I'm doing fine. Thanks!",
  "I've got something interesting to tell you",
  "Once upon a time, I requested to join a chat",
  "However the chat group didn't receive my request",
  "So, I was wondering if you could tell me what happened.",
  "I'm really upset and can't believe it",
  "Let's help each other out.",
  "And sure, I'll do the same for you",
  "Ok, talk to you soon."
}

function on_Hello(peer_id, m)
  log("HELLO", "Hello from peer " .. tostring(peer_id) .. " name: " .. m.name)
  peers[peer_id].name = m.name
end

function on_Msg(peer_id, m)
  hash = sha3(m.author .. m.msg)
  if msgs[hash] == nil then
    msgs[hash] = m.msg
    msg = string.format("<%s>: %s [FROM %s]", m.author, m.msg, peers[peer_id].name)
    log("CHAT", msg)
    gossip(peer_id, m)
  else
    log("CHAT", "IGNORING [Already received]" .. msg)
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
