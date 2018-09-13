epoch = 0

pending_reservations = {}
pending_cancellations = {}

-- TODO: Should use these functions instead

function __on_CreateReservation(peer_id, reservation)
  table.insert(pending_reservations, reservation)
end

function __on_CancelReservation(peer_id, cancellation)
  table.insert(pending_cancellations, cancellation)
end

function broadcast(msg)
  gossip(0, msg)
end

function update_state(time)
  if #pending_reservations + #pending_cancellations > 0 then
    if our_slot == (epoch % TOTAL_VALIDATORS + 1) then
      epoch = epoc + 1;
      st = StateTransition()
      st.epoch = epoch;

      -- TODO: filter out invalid transactions

      for _, r in ipairs(pending_reservations) do
        st.reservations = r
      end

      for _, c in ipairs(pending_cancellations) do
        st.cancellations = c
      end

      to_sign = st:serialize()
      st.signature = secp256k1_sign(private_key, to_sign)
      broadcast(st)

      -- Reset mempool
      pending_reservations = {}
      pending_cancellations = {}
    end
  end
end
