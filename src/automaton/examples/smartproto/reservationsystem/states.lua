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
validators["size"] = 1
