function update(time)
  -- print("======== SIZE " .. tostring(validators["size"]) )
  if validators["size"] == TOTAL_VALIDATORS then
  end
end

function sent(peer_id, msg_id, success)
end

function on_RegisterValidators(peer_id, msg)
  local size = validators["size"]
  for k,v in pairs(msg.public_key) do
    if validators[v] == nil then
      -- print(nodeid .. " RECEIVED " .. hex(v))
      validators[v] = 1
      validators["size"] = validators["size"] + 1
    end
  end
  if size < validators["size"] then
    gossip(peer_id, validators_msg())
  end
  if validators["size"] == TOTAL_VALIDATORS then
    for k in pairs(validators) then
      -- TODO(NOW): put all validators in a table validators_sorted and sort them
    end
  end
end

function on_StateTransition(peer_id, state_transition)
end

function on_CreateReservation(peer_id, reservation)
end

function on_CancelReservation(peer_id, cancellation)
end

function debug_html()
  return ""
end
