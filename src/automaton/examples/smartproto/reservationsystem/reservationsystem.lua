function update(time)
  if table_length(validators) == TOTAL_VALIDATORS then
    update_sate(time)
  end
end

function sent(peer_id, msg_id, success)
end

function on_RegisterValidators(peer_id, msg)
  log("on_RegisterValidators", "call by peer: " .. tostring(peer_id))
  local new_validator = false
  for k,v in pairs(msg.public_key) do
    if validators[v] == nil then
      log("new validator from ", nodeid .. " RECEIVED " .. hex(v))
      validators[v] = 1
      new_validator = true
    end
  end
  if new_validator then
    gossip(peer_id, validators_msg())
    if table_length(validators) == TOTAL_VALIDATORS then
      for k, v in pairs(validators) do
        table.insert(validators_sorted, k)
      end
      table.sort(validators_sorted)
      --log("total_validators ", validators_sorted)
      for i,n in ipairs(validators_sorted) do
        log("validators", tostring(i) .. ": " .. hex(n))
        if public_key == n then
          our_slot = i
          log("validators", tostring(i) .. " IS OUR SLOT!")
        end
      end
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
