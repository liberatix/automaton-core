-- mining helper

ITERATIONS = 1000000

nonce = {0}

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

for i = 1, 65537 do
  inc_nonce(nonce)
  print(hex(nonce_str(nonce)))
end

t = os.clock()

for i = 1, ITERATIONS do
  inc_nonce(nonce)
end
print(hex(nonce_str(nonce)))

t = os.clock() - t
print(string.format("nonce inc [%.3f M/s]", ITERATIONS / t / 1000000))


-- Node initialization
function init()
end

function onBlockHeader(sender, msg)
  hash = msg.hash
  sender.blocks[msg.hash] = msg

  if sender.blocks[msg.prev_hash] == nil then
    -- Since we don't know anything about prior block from this peer
    sender:send(GetHeaders({hash=msg.prev_hash, count=10}))
  else

  end

  for i = 1,#peers do
    peer = peers[i]
    if peer.height < msg.height then
      peer.send(msg)
    end
  end
end

print("Loading Blockchain Smart Protocol...")
x = 5
print(hex(sha3(abc)))

blocks = {}

m = Block()

m.miner = "test"

print(hex(m:serialize()))

t = os.clock()
for i = 1, ITERATIONS do
  m.miner = i
--  blocks[#blocks + 1] = m:serialize()
end
t = os.clock() - t
print(string.format("create/set_name/serialize [%.3f M/s]", ITERATIONS / t / 1000000))

t = os.clock()
for i = 1, ITERATIONS do
  m:set_blob(1, i)
--  blocks[#blocks + 1] = m:serialize()
end
t = os.clock() - t
print(string.format("create/set_id/serialize [%.3f M/s]", ITERATIONS / t / 1000000))

print(m.miner)
print(#blocks)
