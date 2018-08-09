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


-- validate and add the block if we have it
function validateBlock(block)
  -- get the block hash
  hash = blockHash(block)
  print("validating block with hash: " .. hex(hash))

  -- Check if we already have the block
  if blocks[hash] ~= nil then
    --print (BLOCK.DUPLICATE)
    return BLOCK.DUPLICATE
  end
  -- Check if we have the previous block
  if block.prev_hash ~= GENESIS_HASH and blocks[block.prev_hash] == nil then
    -- TODO(Samir): Currenly ignoring blocks with no parent, we will not sync if we are out of sync
    return BLOCK.NO_PARENT
  end
  -- TODO(Samir): Check if hash is less than difficulty
  local target = get_target(difficulty)
  if hash > target then
    print "In validateBlock, target > hash"
    return BLOCK.INVALID
  end
  return BLOCK.VALID
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
  -- TODO(Samir): Instead of checking block validity handle the cases
  print("Block Validity: " .. block_validity)
  -- check if we have it
  -- validate block
  -- if valid, shout

  --===========================

    -- Is the block part of the longest chain?
    if block.height == #blockchain then

    --if block.prev_hash == "" and #blockchain == 0 then
    --  blockchain[#blockchain] = block
    --  blocks[hash] = block
    --else

    --else if block.prev_hash == blockchain[block.height - 1] then
    end
    -- get block hash

    -- block is valid let's add it
    blocks[hash] = block

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
      --print(hex(block_hash))
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
  local prev_hash = blockchain[#blockchain-1] or GENESIS_HASH
  local found, block = mine(sha3("Samir"), prev_hash, #blockchain, nonce, 1000)
  -- if a block is mined call broadcast to all peers
  if found then
    print(block)
    onBlock(-1, block)
    --local block_validity = validateBlock(block)
  end
  i = i+1
end


init()
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





-- ===============================================================================================
-- ===============================================================================================
-- ===============================================================================================

ITERATIONS = 1000000

-- for i = 1, 65537 do
--   inc_nonce(nonce)
--   print(hex(nonce_str(nonce)))
-- end
--
-- t = os.clock()
--
-- for i = 1, ITERATIONS do
--   inc_nonce(nonce)
-- end
-- print(hex(nonce_str(nonce)))
--
-- t = os.clock() - t
-- print(string.format("nonce inc [%.3f M/s]", ITERATIONS / t / 1000000))
--
--
-- -- Node initialization
-- --function init()
-- --end
--
-- function onBlockHeader(sender, msg)
--   hash = msg.hash
--   sender.blocks[msg.hash] = msg
--
--   if sender.blocks[msg.prev_hash] == nil then
--     -- Since we don't know anything about prior block from this peer
--     sender:send(GetHeaders({hash=msg.prev_hash, count=10}))
--   else
--
--   end
--
--   for i = 1,#peers do
--     peer = peers[i]
--     if peer.height < msg.height then
--       peer.send(msg)
--     end
--   end
-- end
--
-- print("Loading Blockchain Smart Protocol...")
-- x = 5
-- print(hex(sha3(abc)))
--
-- blocks = {}
--
-- m = Block()
--
-- m.miner = "test"
--
-- print(hex(m:serialize()))
--
-- t = os.clock()
-- for i = 1, ITERATIONS do
--   m.miner = i
-- --  blocks[#blocks + 1] = m:serialize()
-- end
-- t = os.clock() - t
-- print(string.format("create/set_name/serialize [%.3f M/s]", ITERATIONS / t / 1000000))
--
-- t = os.clock()
-- for i = 1, ITERATIONS do
--   m:set_blob(1, i)
-- --  blocks[#blocks + 1] = m:serialize()
-- end
-- t = os.clock() - t
-- print(string.format("create/set_id/serialize [%.3f M/s]", ITERATIONS / t / 1000000))
--
-- print(m.miner)
-- print(#blocks)
