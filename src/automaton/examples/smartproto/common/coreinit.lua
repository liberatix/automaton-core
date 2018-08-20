-- coreinit.lua

-- HISTORY

history_add("dump_logs()");

history_add("testnet(simulation, blockchain_node, 10, 1)")
history_add("testnet(localhost, blockchain_node, 10, 1)")

-- SMART PROTOCOLS FACTORY FUNCTIONS

function blank(id)
  return node(id, {}, {}, {})
end

function blockchain_node(id)
  return node(
    id,
    {"automaton/examples/smartproto/blockchain/blockchain.proto"},
    {
      "automaton/examples/smartproto/blockchain/graph.lua",
      "automaton/examples/smartproto/blockchain/blockchain.lua"
    },
    {"Block", "GetBlocks", "Blocks"}
  )
end

function chat_node(id)
  return node(
    id,
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

-- NAMES FOR TEST NODE IDS

names = {
  "Alice",
  "Bob",
  "Charlie",
  "Dave",
  "Emily",
  "Frank",
  "Gary",
  "Heather",
  "Ivan",
  "Jacob",
  "Kyle",
  "Larry",
  "Mary",
  "Nancy",
  "Oscar",
  "Paul",
  "Rachel",
  "Steven",
  "Tom",
  "Victor",
  "Walter",
  "Zach",

  "Ace",
  "Aaron",
  "Adam",
  "Alexander",
  "Amanda",
  "Amy",
  "Andrew",
  "Angela",
  "Anna",
  "Anthony",
  "Ashley",
  "Barbara",
  "Benjamin",
  "Betty",
  "Brandon",
  "Brenda",
  "Brian",
  "Carol",
  "Charles",
  "Christina",
  "Christine",
  "Christopher",
  "Cynthia",
  "Daniel",
  "Deborah",
  "Debra",
  "Dennis",
  "Donald",
  "Donna",
  "Edward",
  "Elizabeth",
  "Eric",
  "George",
  "Gregory",
  "James",
  "Jason",
  "Jeffrey",
  "Jennifer",
  "Jessica",
  "John",
  "Jonathan",
  "Jordan",
  "Jose",
  "Joseph",
  "Joshua",
  "Julie",
  "Justin",
  "Karen",
  "Katherine",
  "Kathleen",
  "Kelly",
  "Kenneth",
  "Kevin",
  "Kimberly",
  "Laura",
  "Linda",
  "Lisa",
  "Margaret",
  "Maria",
  "Mark",
  "Matthew",
  "Melissa",
  "Michael",
  "Michelle",
  "Nathan",
  "Nicholas",
  "Nicole",
  "Pamela",
  "Patricia",
  "Patrick",
  "Rebecca",
  "Richard",
  "Robert",
  "Ronald",
  "Ryan",
  "Samantha",
  "Samuel",
  "Sandra",
  "Sarah",
  "Scott",
  "Sharon",
  "Stephanie",
  "Stephen",
  "Susan",
  "Thomas",
  "Timothy",
  "Tyler",
  "William",
  "Zachary",
}

-- NETWORK SIMULATION

function sim_bind(i)
  return string.format("sim://5:500:%d", i)
end

function sim_addr(i)
  return string.format("sim://150:1000:500:%d", i)
end

function simulation(node_factory, NODES, PEERS)
  for i = 1, NODES do
    print(sim_bind(i))
    nodes[i] = node_factory(names[i])
    nodes[i]:listen(sim_bind(i))
  end

  for i = 1, NODES do
    for j = 1, PEERS do
      a = ((i + j - 1) % NODES) + 1;
      print(sim_addr(a))
      peer_id = nodes[i]:add_peer(sim_addr(a))
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
    nodes[i] = node_factory(names[i])
    nodes[i]:listen(tcp_addr(i))
  end

  -- connect to peers
  for i = 1, NODES do
    for j = 1, PEERS do
      a = (i + j - 1) % NODES + 1
      peer_id = nodes[i]:add_peer(tcp_addr(a))
      nodes[i]:connect(peer_id)
    end
  end
end

function testnet(discovery, node_factory, num_nodes, num_peers)
  discovery(node_factory, num_nodes, num_peers)
end

-- GLOBALS

nodes = {}

function dump_logs()
  for i = 1, #nodes do
    nodes[i]:dump_logs(string.format("logs/N%03d-%s.html", i, names[i]))
  end
end

--[[
function chat_test()
  setup_localhost(chat_node, 5, 1)
end

function blockchain_test()
  setup_localhost(blockchain_node, 10, 1)
end

]]

--[[
  Runs simulation with specific configuration.

  cfg is a table which can have the following keys:

  nodes
  min_bandwidth
  max_bandwidth
  min_lag
  max_lag
  max_incoming_peers
  max_outgoing_peers
]]
