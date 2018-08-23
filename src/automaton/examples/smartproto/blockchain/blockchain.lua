-- blockchain.lua

current_message_id = 1

last_hash = GENESIS_HASH

-- node callback functions
function update(time)
  local hash = cur_hash()
  if hash ~= last_hash then
    last_hash = hash
    log("update", "Last hash changed to " .. hex(last_hash))
  end

  local found, block = mine(nodeid, hash, #blockchain+1, nonce)

  -- if a block is mined call broadcast to all peers
  if found then
    -- print("Found block")
    on_Block(0, block)
  end

  -- for each peer call the necesery function
  for k, v in pairs(peers) do
    if v.state == STATE.HANDSHAKE then
      handshake(k)
    end
  end
end

function pid(peer_id)
  if conn[peer_id] ~= nil then
    return string.format("CONN%d[%s]", peer_id, conn[peer_id].name)
  end
  return "N/A"
end

function sent(peer_id, msg_id, success)
  if success then
    -- log(pid(peer_id), "Successfully sent message id: " .. tostring(msg_id))
  else
    peers[peer_id] = nil
    log(pid(peer_id), "Error sending message id: " .. tostring(msg_id))
  end
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
  local target = get_target()

  -- Check if we already have the block
  if blocks[hash] ~= nil then
    log_block("validateBlock", block, "DUPLICATE")
    return BLOCK.DUPLICATE
  -- Check if hash is greater than difficulty
  elseif hash > target then
    log_block("validateBlock", block, "INVALID target > hash")
    return BLOCK.INVALID
  -- block height can't be less than one
  elseif block.height < 1 then
    log_block("validateBlock", block, "INVALID height < 1")
    return BLOCK.INVALID
  -- The block should have its predecessor in blocks unless it is the first block
  elseif block.prev_hash ~= GENESIS_HASH and blocks[block.prev_hash] == nil then
    log_block("validateBlock", block, "NO_PARENT")
    return BLOCK.NO_PARENT
  -- 1. If this is the first block, it needs to have GENESIS_HASH as prev_hash.
  -- 2. If it is not the first block, check if the height of
  --    the block with hash prev_hash is the height of this
  elseif block.height == 1 and block.prev_hash ~= GENESIS_HASH
      or block.height > 1 and blocks[block.prev_hash].height ~= block.height-1 then
    log_block("validateBlock", block, "INVALID 2")
    log("validateBlock", "INVALID 2")
    return BLOCK.INVALID
  else
    log_block("validateBlock", block, "VALID")
    return BLOCK.VALID
  end
end

function sendBlock(peer_id, blockHash) -- TODO(Samir): Use sendBlock, for genesis give proper hash
  -- TODO(Samir): Implement block sending to other peers, Check if the block is received
  --print ("sending block " .. hex(blockHash))

  current_message_id = current_message_id + 1
  -- log(pid(peer_id), "Sending the following  block to this peer -- " .. tostring(current_message_id))
  if blockHash == GENESIS_HASH then
    local no_blocks = Block()
    no_blocks.height = 0
    no_blocks.miner = "No miner"
    no_blocks.prev_hash = GENESIS_HASH
    no_blocks.nonce = ""
    log_block(pid(peer_id), no_blocks)
    send(peer_id, no_blocks, current_message_id)
  else
    log_block(pid(peer_id), blocks[blockHash])
    local block = Block()
    block.miner = blocks[blockHash].miner
    block.prev_hash = blocks[blockHash].prev_hash
    block.height = blocks[blockHash].height
    block.nonce = blocks[blockHash].nonce
    send(peer_id, block, current_message_id)
  end
end

function peer_connected(peer_id)
  log("connected", "Connected to " .. tostring(peer_id))
  log(pid(peer_id), "Connected!")
  peers[peer_id] = {}
  peers[peer_id].state = STATE.HANDSHAKE
  log(pid(peer_id), "STATE: HANDSHAKE")

  -- peer -> {IP = "0.0.0.0", state = STATE.HANDSHAKE, sent_block_hash = nil received_block = nil}},
  -- save peer to peer list -- set peer state
  -- send highest block or create and send block with height 0 to indicate we have no blocks
  -- TODO(Samir): peers[peer_id].sent_block_hash needs to be set when message is confirmed to be received
  if #blockchain > 0 then
    sendBlock(peer_id, blockchain[#blockchain])
    peers[peer_id].sent_block_hash = blockchain[#blockchain]
  else
    sendBlock(peer_id, GENESIS_HASH)
    peers[peer_id].sent_block_hash = GENESIS_HASH
  end
end


function on_Block(peer_id, block)
  if block == nil then
    log("ERRORS", "on_Block called with block = nil!!!")
    return
  end
  if block.height == 0 then
    log(pid(peer_id), "Received a block with height 0 (This peer has no blocks)")
  else
    log_block("on_Block", block)
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
  -- log("on_Block", " Block Validity: " .. block_validity)
  -- log("validateBlock", " Block Validity: " .. block_validity)
  if block_validity == BLOCK.VALID  then
    -- log("on_Block", " Valid block added to blocks")
    blocks[hash] = {
      miner = block.miner,
      prev_hash = block.prev_hash,
      height = block.height,
      nonce = block.nonce
    }
    gossip(peer_id, hash)
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

function gossip(from, block_hash)
  for k, v in pairs(peers) do
    -- TODO(Samir): Decide to which peer states we should send the block
    --if v.state == STATE.IN_CONSENSUS then
      if k ~= from then
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

function log_block(identifer, block, extra)
  log(identifer,
    string.format("%s | %04d | %12s | %s",
      hex(blockHash(block)),
      block.height,
      block.miner,
      extra))
end
