
function update(time)
  -- print("Update called at", time)
  -- for each peer check to see if we need to send more info, close connection, etc.
  print("initial peer state: ")
  print(tprint(peers))
  print("got here")
  for k, v in pairs(peers) do
    print(k)
    print(tprint(v))
  end
  -- For each peer with state HANDSHAKE:
    -- Start the HANDSHAKE to find out if we are
end



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

-- Initial peer list
initial_peers = {
  {IP = "0.0.0.0", STATE = "NOT_CONNECTED"},
  {IP = "1.1.1.1", STATE = "NOT_CONNECTED"},
  {IP = "2.2.2.2", STATE = "NOT_CONNECTED"},
  {IP = "3.3.3.3", STATE = "NOT_CONNECTED"},
}

peers = {}
blocks = {}
blockchain = {}

difficulty = {}
difficulty.leadingZeros = 1
difficulty.prefix = "FFFFFF"


-- mining helper
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
  peers = initial_peers
  -- Load the blockchain if avalible
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
  -- TODO(Samir): Instead of checking block validity handle the cases
  print("Block Validity: " .. block_validity)
  -- Valid block is a block that:
  -- 1. Is a new block, with valid hash and height >= 1
  -- 2. Either prev_hash is in blocks or the block is with height #1 and prev_hash is GENESIS_HASH
  -- 3. Has height difference of one with the blocks[prev_hash].height
  --    or it is block#1 with prev_hash equal to GENESIS_HASH
  -- If it is VALID
  if block_validity == BLOCK.VALID  then
    print "Valid block added to blocks"
    blocks[hash] = block
    --Add the block to the head of the blockchain if possobile
    --! if #blockchain == 0 or block.prev_hash == blockchain[#blockchain] then
    --!   print "block added to the longest chain"
    --!   blockchain[#blockchain+1] = hash
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

function onBlocks(peer_id, msg)

end

function onGetBlocks(peer_id, msg)

end

-- Takes in block with miner, prev_hash, height
--
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


init()


--===== Helper functions for debuging =============================================
-- Print table in human readable format
function tprint (tbl, indent)
  if not indent then indent = 0 end
  for k, v in pairs(tbl) do
    formatting = string.rep("  ", indent) .. k .. ": "
    if type(v) == "table" then
      print(formatting)
      tprint(v, indent+1)
    elseif type(v) == 'boolean' then
      print(formatting .. tostring(v))
    else
      print(formatting .. v)
    end
  end
end


--================================== MAIN =========================================
i = 0
while i < 2 do
  local nonce = {0}
  target = get_target(difficulty)
  local prev_hash = blockchain[#blockchain] or GENESIS_HASH
  --TODO(Samir): put miner, perv_hash, #blockchain+1 and nonce in a struct
  --             and just previous BLOCK hash instead
  local found, block = mine(sha3("Samir"), prev_hash, #blockchain+1, nonce, 1000)
  -- if a block is mined call broadcast to all peers
  if found then
    onBlock(-1, block)
    --local block_validity = validateBlock(block)
  end
  i = i+1
end





--===== Not in use, will be deleted when no longer needed =============================
function tests()
  -- FOR TESTING ONLY, REMOVE AFTER
  nonce = {0}
  genesis = Block()
  genesis.miner = sha3("automaton")
  genesis.prev_hash = ""
  genesis.height = 123
  inc_nonce(nonce)
  genesis.nonce = nonce_str(nonce)
  --blocks[blockHash(genesis)] = genesis
  validateBlock(genesis)
end
