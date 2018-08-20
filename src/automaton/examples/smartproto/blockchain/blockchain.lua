nonce = {105}

current_message_id = 1
-- node callback functions
function update(time)
  sendBlock(1, GENESIS_HASH)
  -- log("update", string.format("Update called at %d", time))
  log("update", string.format("UPDATE STARTED for node %s", node_id))
  log("Blockchain", "Height: " .. tostring(#blockchain))
  log("Blockchain", "Last Hash: " .. tostring(hex(blockchain[#blockchain] or GENESIS_HASH)))

  local prev_hash = blockchain[#blockchain] or GENESIS_HASH
  local found, block = mine(nodeid, prev_hash, #blockchain+1, nonce, 1000)
  -- if a block is mined call broadcast to all peers
  if found then
    on_Block(-1, block)
  end

  -- for each peer call the necesery function
  for k, v in pairs(peers) do
    if v.state == STATE.HANDSHAKE then
      handshake(k)
    end
  end
end

function pid(id)
  return "PEER " .. tostring(id)
end

function sent(peer_id, msg_id, success)
  -- print "GOT TO SENT"
  -- print "printing the inputs"
  -- print ("peer_id: ", peer_id)
  -- print (" msg_id: " .. msg_id)
  -- pritn (" succsess: " .. success)
  -- print ("inputs printed")
  if success then
    log(pid(peer_id), "Successfully sent message id: " .. tostring(msg_id))
  else
    peers[peer_id] = nil
    log(pid(peer_id), "Error sending message id: " .. tostring(msg_id))
  end
end

function disconnected(peer_id)
  log(pid(peer_id), "Disconnected (in script)")
end


-- mining helper

-- states
STATE = {}
STATE.HANDSHAKE = 1
STATE.BEHIND = 2
STATE.AHEAD = 3
STATE.DISCONECTED = 4
STATE.NOT_CONNECTED = 5
STATE.IN_CONSENSUS = 6

-- block state after validation
BLOCK = {}
BLOCK.VALID = 1
BLOCK.INVALID = 2
BLOCK.DUPLICATE = 3
BLOCK.NO_PARENT = 4

GENESIS_HASH = sha3("automaton")

peers = {}
blocks = {}
blockchain = {}

difficulty = {}
difficulty.leadingZeros = 1
difficulty.prefix = "03FFFF"


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
  blockdata = tostring(block.miner) .. tostring(block.prev_hash) .. tostring(block.height) .. tostring(block.nonce);
  return sha3(blockdata)
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

  log("validateBlock", "validating block: ")
  log_block("validateBlock", block)

  -- Check if we already have the block
  if blocks[hash] ~= nil then
    log("validateBlock", "DUPLICATE")
    return BLOCK.DUPLICATE
  -- Check if hash is greater than difficulty
  elseif hash > target then
    log("validateBlock", "In validateBlock, target > hash")
    return BLOCK.INVALID
  -- block height can't be less than one
  elseif block.height < 1 then
    log("validateBlock", "INVALID")
    return BLOCK.INVALID
  -- The block should have its predecessor in blocks unless it is the first block
  elseif block.prev_hash ~= GENESIS_HASH and blocks[block.prev_hash] == nil then
    log("validateBlock", "NO_PARENT")
    return BLOCK.NO_PARENT
  -- 1. If this is the first block, it needs to have GENESIS_HASH as prev_hash.
  -- 2. If it is not the first block, check if the height of
  --    the block with hash prev_hash is the height of this
  elseif block.height == 1 and block.prev_hash ~= GENESIS_HASH
      or block.height > 1 and blocks[block.prev_hash].height ~= block.height-1 then
    log("validateBlock", "INVALID 2")
    return BLOCK.INVALID
  else
    log("validateBlock", "VALID")
    return BLOCK.VALID
  end
end

function sendBlock(peer_id, blockHash) -- TODO(Samir): Use sendBlock, for genesis give proper hash
  -- TODO(Samir): Implement block sending to other peers, Check if the block is received
  --print ("sending block " .. hex(blockHash))

  current_message_id = current_message_id + 1
  log(pid(peer_id), "Sending the following  block to this peer -- " .. tostring(current_message_id))
  if blockHash == GENESIS_HASH then
    local no_blocks = Block()
    no_blocks.height = 0
    no_blocks.miner = "No miner"
    no_blocks.prev_hash = GENESIS_HASH
    no_blocks.nonce = ""
    log_block(pid(peer_id), no_blocks)
    send(peer_id, no_blocks, current_message_id)
  else
    log_block("CRASH " .. pid(peer_id), blocks[blockHash])
    local block = Block()
    block.miner = blocks[blockHash].miner
    block.prev_hash = blocks[blockHash].prev_hash
    block.height = blocks[blockHash].height
    block.nonce = blocks[blockHash].nonce
    send(peer_id, block, current_message_id)
  end
end

function connected(peer_id)
  log("connected", "Connected to " .. tostring(peer_id))
  log(pid(peer_id), "Connected!")
  peers[peer_id] = {}
  peers[peer_id].state = STATE.HANDSHAKE
  log(pid(peer_id), "STATE: HANDSHAKE")

  --b = Block()
  --b.miner = "Ace"
  --send(1, b, 0) -- send (peer_id, message, mesage_id)
  -- peer -> {IP = "0.0.0.0", state = STATE.HANDSHAKE, sent_block_hash = nil received_block = nil}},
  -- save peer to peer list -- set peer state
  -- send highest block or create and send block with height 0 to indicate we have no blocks
  -- TODO(Samir): peers[peer_id].sent_block_hash needs to be set when message is confirmed to be received
  if #blockchain > 0 then
    sendBlock(peer_id, blockchain[#blockchain])
    peers[peer_id].sent_block_hash = blockchain[#blockchain]
  else
    sendBlock(peer_id, GENESIS_HASH)
    sendBlock(peer_id, GENESIS_HASH)
    peers[peer_id].sent_block_hash = GENESIS_HASH
  end
end


function on_Block(peer_id, block)
  log("on_Block", "Received a block from: " .. pid(peer_id))
  if block == nil then
    log("on_Block", "WARRNING!!!!! The block is == nil ")
    return
  end
  if block.height == 0 then
    log("on_Block", " Block height 0 (the peer has no blocks)")
    log(pid(peer_id), "Received a block with height 0 (This peer has no blocks)")
  else
    log_block("on_Block", block)
    log(pid(peer_id), "Received the following block from this peer: ")
    log_block(pid(peer_id), block)
  end
  --log(pid(peer_id), "Received a block: " .. tostring(msg_id))
  -- If this is the first block from a newly connect peer
  if peers[peer_id] ~= nil and peers[peer_id].received_block == nil then
    log("on_Block", " First block from peer: " .. peer_id)
    peers[peer_id].received_block = block
    --print(tprint(peers[peer_id]))
    -- if this peer has no block return and handle the rest in handshake
    if block.height == 0 then
      log("on_Block", " in on_Block, we got block with height 0")
      return
    end
  end

  -- Validate, save and broadcast
  local block_validity = validateBlock(block)
  local hash = blockHash(block)
  log("on_Block", " Block Validity: " .. block_validity)
  log("validateBlock", " Block Validity: " .. block_validity)
  if block_validity == BLOCK.VALID  then
    log("on_Block", " Valid block added to blocks")
    blocks[hash] = {
      miner = block.miner,
      prev_hash = block.prev_hash,
      height = block.height,
      nonce = block.nonce
    }
    shout(peer_id, hash)
    --Add the block to the head of the blockchain if possobile
    --! if #blockchain == 0 or block.prev_hash == blockchain[#blockchain] then
    --!   print "block added to the longest chain"
    --!   blockchain[#blockchain+1] = hash
    -- Check if we get a longer chain. Does not matter if it is the main or alternative.
    if block.height == #blockchain+1 then
      -- We are sure that this is the head of the longest chain
      blockchain[#blockchain+1] = hash
      -- Check if blocks[block.prev_hash] is part of the main chain and replace if necesery
      local block_index = (#blockchain)-1
      local longest_chain_hash = block.prev_hash
      while block_index >= 1 and (blockchain[block_index] ~= longest_chain_hash) do
        blockchain[block_index] = longest_chain_hash
        longest_chain_hash = blocks[longest_chain_hash].prev_hash
        block_index = block_index - 1
      end
    end
  -- If it is DUPLICATE
  elseif(block_validity == BLOCK.DUPLICATE) then
    -- do nothing
  -- If it is INVALIDd
  elseif(block_validity == BLOCK.INVALID) then
    -- do nothing or respond with invalid
  -- if there is no block with prev_hash in blocks
  elseif(block_validity == BLOCK.NO_PARENT) then
    -- potencial fork, the block will not be saved
    -- find out which blocks we need to request or just request from start
  end
end

function shout(from, block_hash)
  log("shout", "shouting, from = " .. tostring(from))
  --print(tprint(peers))
  for k, v in pairs(peers) do
    -- TODO(Samir): Decide to which peer states we should send the block
    --if v.state == STATE.IN_CONSENSUS then
      if k ~= from then
        log("shout", "Sending to peer: " .. tostring(k))
        --print(blocks[block_hash])
        -- current_message_id = current_message_id+1
        -- send(k, blocks[block_hash], current_message_id)
        sendBlock(k, block_hash)
      end
  --end
  end
end

function onBlocks(peer_id, msg)
end

function onGetBlocks(peer_id, msg)
end

function handshake(peer_id)
  --print ("start handshake with peer_id: " .. peer_id)
  -- print(peers[peer_id].received_block.miner)
  -- print(peers[peer_id].received_block.height)
  -- print(tprint(peers[peer_id]))
  -- Check if peer exists in peers
  if peer_id <= 0 or peers[peer_id] == nil then
    log("handshake", "ERROR(Samir 301): in handshake,"
    .. "peer_id needs to be possitive number and peers[peer_id] should not be nil")
    return
  end
  if peers[peer_id].state == STATE.HANDSHAKE and peers[peer_id].received_block ~= nil then
    --print "we have sent and recieved block"
    -- if block headers are the same
      -- Consensus
    -- elseif we have the decieved_block
      -- Send all blocks after the recieved blocks
    -- elseif we have the longest chain
      -- send all blocks from begining
    -- else get blocks, validate and save until consesnsus is reached
  end




  -- if peers[peer_id].STATE == STATE.HANDSHAKE and peers[peer_id].sent_block_hash == nil then
  --   if #blockchain == 0 then
  --     local no_blocks = block()
  --     no_blocks.height = 0
  --     send(peer_id, no_blocks, 0)
  --   else
  --     sendBlock(peer_id, blockchain[#blockchain])
  --   end
  -- elseif true then
  --   --print "elseif block is sent"
  --   print(tprint(peers[peer_id]))
  -- end
end

-- Takes in block with miner, prev_hash, height
--
function mine(miner, prev_hash, height, nonce, attempts)
  local target = get_target(difficulty)
  local block_data = tostring(miner) .. tostring(prev_hash) .. tostring(height)
  for i = 0, attempts do
    block_hash = sha3(block_data .. nonce_str(nonce))
    --print(hex(block_hash))
    if block_hash <= target then
      -- create and return block
      mined_block = Block()
      mined_block.miner = miner
      mined_block.prev_hash = prev_hash
      mined_block.height = height
      mined_block.nonce = nonce_str(nonce)
      log("miner", "Just mined the following block:")
      log_block("miner", mined_block)
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

function log_block(identifer, block)
  log(identifer, " height: " .. tostring(block.height))
  log(identifer, " Hash: " .. hex(blockHash(block)))
  log(identifer, " prev_hash: " .. hex(tostring(block.prev_hash)))
  log(identifer, " miner: " .. hex(tostring(block.miner)))
end
