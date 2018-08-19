names = {
  "Alice",
  "Bob",
  "Chalie",
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
  "Emily",
  "Eric",
  "Gary",
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
  "Larry",
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
  "Rachel",
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

-- LOCALHOST DISCOVERY SETUP

math.randomseed(os.time())
START_PORT = 5000 + math.random(60000)

function tcp_addr(i)
  return "tcp://127.0.0.1:" .. tostring(START_PORT + i)
end

function setup_localhost(NODES, PEERS, node_factory)
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

-- GLOBALS

nodes = {}

function dump_logs()
  for i = 1, #nodes do
    nodes[i]:dump_logs(string.format("logs/N%d-%s.html", i, names[i]))
  end
end

function chat_test()
  setup_localhost(5, 1, chat_node)
end

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

function run_simulation(cfg)
end

function run_localhost(cfg)
end

function run_tcp(cfg)
end

function sim_test()
  NODES = 100
  PEERS = 4

  a={}

  for i = 1, NODES do
    a[i] = string.format("sim://5:5:%d", i)
    nodes[i] = anode(names[i])
    nodes[i]:listen(a[i])
  end

  for i = 1, NODES do
    for j = 1, PEERS do
      paddr = ((i + j - 1) % NODES) + 1;
      peer_id = nodes[i]:add_peer(string.format("sim://150:1000:4:%d", paddr))
      nodes[i]:connect(peer_id)
    end
  end
end

function tcp_test()
  N = 5
  M = 1

  for i = 1, N do
    nodes[i] = anode(names[i])
    nodes[i]:listen(tcp_addr(i))
  end

  for i = 1, N do
    for j = 1, M do
      a = (i + j - 1) % N + 1
      -- print(tcp_addr(i), "->", tcp_addr(a))
      peer_id = nodes[i]:add_peer(tcp_addr(a))
      nodes[i]:connect(peer_id)
    end
  end
end
