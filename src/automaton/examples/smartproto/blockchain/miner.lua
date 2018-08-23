-- miner.lua

-- miner attempts per update
MINE_ATTEMPTS = 100

nonce = {0}

difficulty = {}
difficulty.leadingZeros = 1
difficulty.prefix = "3FFFFF"

-- Increments nonce, expands nonce size if necessary
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

-- Converts nonce into a binary string (Lua lstring)
function nonce_str(n)
  s = {}
  for i = 1, #n do
    table.insert(s, string.char(n[i]))
  end
  return table.concat(s)
end

function get_target()
  return bin(string.rep("00", difficulty.leadingZeros) .. difficulty.prefix ..
    string.rep("00", 32-difficulty.leadingZeros-3))
end

function mine_block(block_data, nonce, target_hash, attempts)
  for i = 1, attempts do
    block_hash = sha3(block_data .. nonce_str(nonce))
    if block_hash <= target_hash then
      return true
    else
      inc_nonce(nonce)
    end
  end

  return false
end

-- Takes in block with miner, prev_hash, height
function mine(miner, prev_hash, height, nonce, attempts)
  -- if true then return false end

  local target = get_target(difficulty)
  local block_data = tostring(miner) .. tostring(prev_hash) .. tostring(height)
  -- mine_block
  if mine_block(block_data, nonce, target, MINE_ATTEMPTS) then
    -- create and return block
    mined_block = Block()
    mined_block.miner = miner
    mined_block.prev_hash = prev_hash
    mined_block.height = height
    mined_block.nonce = nonce_str(nonce)
    return true, mined_block
  end

  return false
end
