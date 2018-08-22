-- coreinit.lua

-- GLOBALS

nodes = {}
miners = {}
connections_graph_nodes = {}
connections_graph_edges = {}
-- HISTORY


history_add("Alice.set_mining_power(0)")
history_add("testnet(localhost, chat_node, 5, 1)")
history_add("testnet(localhost, chat_node, 10, 1)")

history_add("testnet(simulation, blockchain_node, 3, 1)")
history_add("testnet(simulation, blockchain_node, 10, 1)")

history_add("testnet(simulation, chat_node, 3, 1)")

history_add("set_listeners(5001, 5050)")
history_add("set_listeners(5051, 5100) add_peers('127.0.0.1', 5001, 5050) nIndex = 10")
history_add("testnet(manual, blockchain_node, 0, 0)")

history_add("dump_logs()");
history_add("testnet(localhost, blockchain_node, 100, 1)")

history_add("dump_connections_graph()");

-- SMART PROTOCOLS FACTORY FUNCTIONS

function blank(id)
  return node(id, {}, {}, {})
end

function blockchain_node(id)
  local n = node(
    id,
    100,
    {"automaton/examples/smartproto/blockchain/blockchain.proto"},
    {
      "automaton/examples/smartproto/blockchain/connections.lua",
      "automaton/examples/smartproto/blockchain/graph.lua",
      "automaton/examples/smartproto/blockchain/states.lua",
      "automaton/examples/smartproto/blockchain/miner.lua",
      "automaton/examples/smartproto/blockchain/blockchain.lua"
    },
    {"Block", "GetBlocks", "Blocks"}
  )

  print(id)
  _G[id] = {
    set_mining_power = function(x)
      n:script("MINE_ATTEMPTS=" .. x .. " return ''")
    end,

    mine_block = function(x)
      n:script("mine_block_from_hash(" .. x .. ")")
    end
  }

  return n
end

function chat_node(id)
  return node(
    id,
    100,
    {
      "automaton/examples/smartproto/chat/chat.proto"
    },
    {
      "automaton/examples/smartproto/chat/messages.lua",
      "automaton/examples/smartproto/chat/connections.lua",
      "automaton/examples/smartproto/chat/chat.lua"},
    {"Hello", "Msg"}
  )
end

-- NETWORK SIMULATION DISCOVERY

function sim_bind(i)
  return string.format("sim://5:500:%d", i)
end

function sim_addr(i)
  return string.format("sim://150:1000:500:%d", i)
end

function simulation(node_factory, NODES, PEERS)
  for i = 1, NODES do
    print(sim_bind(i))
    nodes[i] = add_node(node_factory, i)
    nodes[i]:listen(sim_bind(i))
  end

  for i = 1, NODES do
    for j = 1, PEERS do
      a = ((i + j - 1) % NODES) + 1;
      print(sim_addr(a))
      peer_id = add_peer(i, sim_addr(a), a)
      nodes[i]:connect(peer_id)
    end
  end
end

-- LOCALHOST DISCOVERY SETUP

math.randomseed(os.time())
START_PORT = 5000 + math.random(60000)

function tcp_addr(i)
  return "tcp://127.0.0.1:" .. tostring(START_PORT + i)
end

function localhost(node_factory, NODES, PEERS)
  -- create nodes and start listening
  for i = 1, NODES do
    nodes[i] = add_node(node_factory, i)
    nodes[i]:listen(tcp_addr(i))
  end

  -- connect to peers
  for i = 1, NODES do
    local r_peers = {}
    for j = 1, PEERS do
      -- a = (i + j - 1) % NODES + 1
      repeat
        a = math.random(NODES)
      until (a ~= i) and (r_peers[a] == nil)
      r_peers[a] = true

      peer_id = add_peer(i, tcp_addr(a), a)
      nodes[i]:connect(peer_id)
    end
  end
end

-- MANUAL SETUP

manual_listeners = {}
remote_peers = {}

function set_listeners(s, e)
  manual_listeners = {}
  for port = s, e do
    table.insert(manual_listeners, string.format("tcp://127.0.0.1:%d", port))
  end
end

function add_peers(address, s, e)
  for port = s, e do
    table.insert(remote_peers, string.format("tcp://%s:%d", address, port))
  end
end

function manual(node_factory, nn, PEERS)
  local NODES = #manual_listeners

  -- create nodes and start listening
  for i = 1, #manual_listeners do
    nodes[nIndex + i] = add_node(node_factory, nIndex + i)
    nodes[nIndex + i]:listen(manual_listeners[i])
  end

  -- connect to peers
  for i = 1, NODES do
    local r_peers = {}
    for j = 1, PEERS do
      repeat
        a = math.random(#remote_peers)
      until r_peers[a] == nil
      r_peers[a] = true

      peer_id = add_peer(nIndex + i, remote_peers[a], a)
      nodes[nIndex + i]:connect(peer_id)
    end
  end
end

function testnet(discovery, node_factory, num_nodes, num_peers)
  discovery(node_factory, num_nodes, num_peers)
end

function add_node(node_factory, node_id)
  local name = names[node_id]
  local new_node = node_factory(name)
  local s = string.format("{id: '%s', shape: 'box', label: '%s'}", node_id, name)
  table.insert(connections_graph_nodes, s)
  return new_node
end

function add_peer(node_id, address, pid)
  local peer_id = nodes[node_id]:add_peer(address)
  local s = string.format("{from: '%s', to: '%s', arrows:'to'}", node_id, pid)
  table.insert(connections_graph_edges, s)
  return peer_id
end

function dump_logs()
  for i in pairs(nodes) do
    nodes[i]:dump_logs(string.format("logs/N%03d-%s.html", i, names[i]))
  end
end

function dump_connections_graph()
  file = io.open ("logs/connections_graph.html", "w+")
  file:write(create_graph_html())
  file:close()
end
