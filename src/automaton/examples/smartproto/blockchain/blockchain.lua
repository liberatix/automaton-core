print("LOADED!")

-- node callback functions

function update(time)
  -- print("Update called at", time)
end

-- mining helper

-- states
STATE = {}
STATE.NOT_CONNECTED = 1
STATE.HANDSHAKE = 2
STATE.BEHIND = 3
STATE.AHEAD = 4
STATE.DISCONECTED = 5
-- block state after validation
BLOCK = {}
BLOCK.VALID = 1
BLOCK.INVALID = 2
BLOCK.DUPLICATE = 3
BLOCK.NO_PARENT = 4

GENESIS_HASH = sha3("automaton")

peer_states = {}
blocks = {}
blockchain = {}

difficulty = {}
difficulty.leadingZeros = 1
difficulty.prefix = "FFFFFF"


function inc_nonce(n)
  for i = 1, #n do
    if n[i] < 255 then
      n[i] = n[i] + 1
      break
    else
      n[i] = 0
      if i == #n then
        table.insert(n, 1)
        return
      end
    end
  end
end

function nonce_str(n)
  s = {}
  for i = 1, #n do
    table.insert(s, string.char(n[i]))
  end
  return table.concat(s)
end

function blockHash(block)
  blockdata = block.miner .. block.prev_hash .. tostring(block.height) .. block.nonce;
  return sha3(blockdata)
end

function init()
  -- Add initial peer list to peer_states
  -- Create the initial state of the blockchain if it is not avalible
end


-- Valid block is a block that:
-- 1. Is a new block, with valid hash and height >= 1
-- 2. Either prev_hash is in blocks or the block is with height #1 and prev_hash is GENESIS_HASH
-- 3. Has height difference of one with the blocks[prev_hash].height
--    or it is block#1 with prev_hash equal to GENESIS_HASH
function validateBlock(block)
  -- get the block hash and target difficulty
  hash = blockHash(block)
  local target = get_target(difficulty)

  print("validating block with hash: " .. hex(hash))
  print("And height: " .. block.height)
  print(block)

  -- Check if we already have the block
  if blocks[hash] ~= nil then
    return BLOCK.DUPLICATE
  -- Check if hash is greater than difficulty
  elseif hash > target then
    print "In validateBlock, target > hash"
    return BLOCK.INVALID
  -- block height can't be less than one
  elseif block.height < 1 then
    return BLOCK.INVALID
  -- The block should have its predecessor in blocks unless it is the first block
  elseif block.prev_hash ~= GENESIS_HASH and blocks[block.prev_hash] == nil then
    return BLOCK.NO_PARENT
  -- 1. If this is the first block, it needs to have GENESIS_HASH as prev_hash.
  -- 2. If it is not the first block, check if the height of
  --    the block with hash prev_hash is the height of this
  elseif block.height == 1 and block.prev_hash ~= GENESIS_HASH
      or block.height > 1 and blocks[block.prev_hash].height ~= block.height-1 then
    return BLOCK.INVALID
  else
    return BLOCK.VALID
  end
end

function onConnect(peer_id)
  peer_states[peer_id] = { state=HANDSHAKE }
  -- set peer state
  -- send initial block
  -- request getblock
end

function onUpdate()
  -- for each peer check to see if we need to send more info, close connection, etc.
end

function onBlock(peer_id, block)
  local block_validity = validateBlock(block)
  local hash = blockHash(block)
  print("Block Validity: " .. block_validity)

  if block_validity == BLOCK.VALID  then
    print "Valid block added to blocks"
    blocks[hash] = block
    -- Check if we get a longer chain. Does not matter if it is the main or alternative.
    if block.height == #blockchain+1 then
      -- We are sure that this is the head of the longest chain
      blockchain[#blockchain+1] = hash
      -- Check if blocks[block.prev_hash] is part of the main chain and replace if necesery
      local block_index = #blockchain-1
      local longest_chain_hash = block.prev_hash
      while block_index >= 1 and blockchain[block_index] ~= longest_chain_hash do
        blockchain[block_index] = longest_chain_hash
        block_index = block_index - 1
        longest_chain_hash = blocks[longest_chain_hash].prev_hash
      end
    end
    -- shout
  -- If it is DUPLICATE
  elseif(block_validity == BLOCK.DUPLICATE) then
    -- do nothing
  -- If it is INVALID
  elseif(block_validity == BLOCK.INVALID) then
    -- do nothing or respond with invalid
  -- if there is no block with prev_hash in blocks
  elseif(block_validity == BLOCK.NO_PARENT) then
    -- potencial fork, the block will not be saved
    -- find out which blocks we need to request or just request from start
  end
end

function on_Block(peer_id, msg)
  print("Received Block!")
  print(msg:to_json())
end

function connected(peer_id)
  print("Connected to " .. tostring(peer_id))
  b = Block()
  b.miner = "Ace"
  send(1, b, 0)
end

function onBlocks(peer_id, msg)

end

function onGetBlocks(peer_id, msg)

end

-- Takes in block with miner, prev_hash, height
function mine(miner, prev_hash, height, nonce, attempts)
  print "Mining block"
  local target = get_target(difficulty)
  local block_data = miner .. prev_hash .. height
  for i = 0, attempts do
    block_hash = sha3(block_data .. nonce_str(nonce))
    if block_hash <= target then
      -- create and return block
      mined_block = Block()
      mined_block.miner = miner
      mined_block.prev_hash = prev_hash
      mined_block.height = height
      mined_block.nonce = nonce_str(nonce)
      return true, mined_block
    else
     inc_nonce(nonce)
    end
  end
  return false
end

function get_target(difficulty)
  return bin(string.rep("00", difficulty.leadingZeros) .. difficulty.prefix ..
    string.rep("00", 32-difficulty.leadingZeros-3))
end


--================================== MAIN =========================================
i = 0
while i < 1 do
  local nonce = {0}
  target = get_target(difficulty)
  print(target)
  local prev_hash = blockchain[#blockchain] or GENESIS_HASH
  --TODO(Samir): put miner, perv_hash, #blockchain+1 and nonce in a struct
  --             and just previous BLOCK hash instead
  local found, block = mine(sha3("Samir"), prev_hash, #blockchain+1, nonce, 1000)
  -- if a block is mined call broadcast to all peers
  if found then
    print(block)
    onBlock(-1, block)
    --local block_validity = validateBlock(block)
  end
  i = i+1
end


init()
