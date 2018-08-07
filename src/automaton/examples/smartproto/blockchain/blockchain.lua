-- Node initialization
function init()
end

function onConnect(peer)
  peer.state = 0
  
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

blocks = {}
ITERATIONS = 100000
t = os.clock()
for i = 1, ITERATIONS do
  m = Block()
  m.miner = tostring(i)
  blocks[#blocks + 1] = m:serialize()
end
print("GC")
collectgarbage()
t = os.clock() - t
print(string.format("create/set/serialize [%.3f M/s]", ITERATIONS / t / 1000000))

print(#blocks)
