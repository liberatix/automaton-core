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
