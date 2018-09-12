ROOM_COUNT = 5
DAYS = 30
rooms = {}
TOTAL_VALIDATORS = 20
validators = {}
validators_sorted = {}
peers = {}

for i = 1, ROOM_COUNT do
  rooms[i] = {}
end

private_key = sha256(nodeid)
public_key = secp256k1_gen_public_key(private_key)
validators[public_key] = 1


function table_length(t)
  local count = 0
  for _ in pairs(t) do
    count = count + 1
  end
  return count
end
