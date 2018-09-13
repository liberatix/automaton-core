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

-- message StateTransition {
--   //  validator_number * rounds
--   uint64 epoch = 1;
--   repeated CreateReservation reservations = 2;
--   repeated CancelReservation cancellations = 3;
--   // sign(epoch, reservations, cancellations) with the key of the validator
--   bytes signature = 4;
-- }

function on_StateTransition(peer_id, state_transition)

end

function on_CreateReservation(peer_id, reservation)
  if not is_valid_signature(reservation) then
    return
  end
  for _,v in pairs(reservation.room_id) do
    if rooms[v] == nil then
      return
    end
    -- Check if there is conflicting reservation in pending_reservations
    -- for i = reservation.start_day, reservation.end_day do
    --   if rooms_local[v][i] then
    --     return
    --   end
    -- end
  end
  -- Mark pending reservation
  -- for _,v in pairs(reservation.room_id) do
  --   for i = reservation.start_day, reservation.end_day do
  --     rooms_local[v][i] = reservation.client_public_key
  --   end
  -- end


  table.insert(pending_reservations, reservation)
  gossip(peer_id, reservation)
end

function on_CancelReservation(peer_id, cancellation)
  if not is_valid_signature(cancellation) then
    return
  end
  for _,v in pairs(cancellation.room_id) do
    if rooms[v] == nil then
      return
    end
    -- for i = cancellation.start_day, cancellation.end_day do
    --   if rooms_local[v][i] ~= cancellation.client_public_key then
    --     return
    --   end
    -- end
  end
  -- Check if there is conflicting reservation in pending reservations
  -- for _,v in pairs(cancellation.room_id) do
  --   for i = cancellation.start_day, cancellation.end_day do
  --     rooms_local[v][i] = nil
  --   end
  -- end

  table.insert(pending_cancellations, cancellation)
  gossip(peer_id, cancellation)
end

function create_tx(tx_type, rooms, start_day, end_day, client_public_key)
  local msg = 0
  if tx_type == "create" then
    msg = CreateReservation()
  elseif tx_type == "cancel" then
    msg = CancelReservation()
  else
    return
  end

  for _,v in pairs(rooms) do
    msg.room_id = v
  end
  msg.start_day = start_day
  msg.end_day = end_day
  msg.client_public_key = client_public_key
  local to_sign = table.concat(rooms, "")
  to_sign = to_sign .. tostring(start_day) .. tostring(end_day)
  msg.client_signature = secp256k1_sign(private_key, to_sign)
  return msg
end

function debug_html()
  return ""
end

function tests()

end


tests()
