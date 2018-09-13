ROOM_COUNT = 5
DAYS = 30
TOTAL_VALIDATORS = 20

rooms = {}
rooms_local = {}
mempool = {}
validators = {}
validators_sorted = {}
peers = {}

for i = 1, ROOM_COUNT do
  rooms[i] = {}
  rooms_local[i] = {}
end

private_key = sha256(nodeid)
public_key = secp256k1_gen_public_key(private_key)
validators[public_key] = 1

function is_valid_signature(tx)
  local msg = table.concat(tx.room_id, "")
  msg = msg .. tostring(tx.start_day) .. tostring(tx.end_day)
  return secp256k1_verify(tx.client_public_key, msg, tx.client_signature)
end

function table_length(t)
  local count = 0
  for _ in pairs(t) do
    count = count + 1
  end
  return count
end
