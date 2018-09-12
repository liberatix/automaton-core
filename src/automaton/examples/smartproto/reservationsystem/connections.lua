current_message_id = 1
function connected(peer_id)
  peers[peer_id] = "connected"
  send(peer_id, validators_msg(), current_message_id)
  current_message_id = current_message_id + 1
end

function disconnected(peer_id)
  peers[peer_id] = "disconnected"
end

function gossip(peer_id, msg)
  for k,v in pairs(peers) do
    if k ~= peer_id and v == "connected" then
      send(k, msg, current_message_id)
      current_message_id = current_message_id + 1
    end
  end
end


function validators_msg()
  local m = RegisterValidators()
  for k in pairs(validators) do
    m.public_key = k
  end
  return m
end
