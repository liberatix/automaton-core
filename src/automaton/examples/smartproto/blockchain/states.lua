-- peer states
STATE = {}
STATE.HANDSHAKE = 1
STATE.BEHIND = 2
STATE.AHEAD = 3
STATE.DISCONECTED = 4
STATE.NOT_CONNECTED = 5
STATE.IN_CONSENSUS = 6

-- block validation results
BLOCK = {}
BLOCK.VALID = 1
BLOCK.INVALID = 2
BLOCK.DUPLICATE = 3
BLOCK.NO_PARENT = 4

GENESIS_HASH = sha3("automaton")

peers = {}
blocks = {}
blockchain = {}

function cur_hash()
  return blockchain[#blockchain] or GENESIS_HASH
end

function get_block(hash)
  if blocks[hash] ~= nil then
    return blocks[hash]
  elseif hash == GENESIS_HASH then
    return {
      height = 0,
      miner = "",
      prev_hash = "",
      nonce = ""
    }
  end
  return 
end

function update_peers()
  for k, v in pairs(peers) do
    if v.state == STATE.HANDSHAKE then
      handshake(k)
    end
  end
end
