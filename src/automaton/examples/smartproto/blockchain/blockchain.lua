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

ITERATIONS = 1000000
blocks = {}

m = Block()
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
