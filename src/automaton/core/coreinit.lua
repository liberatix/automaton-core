-- GLOBALS

nodes = {}

function dump_logs()
  for i = 1, #nodes do
    nodes[i]:dump_logs(string.format("logs/N%d.html", i))
  end
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

function sim_test2()
  print("================== SIMULATION ==================")

  NODES = 100
  PEERS = 16

  a={}
  n={}

  for i = 1, NODES do
    a[i] = string.format("sim://5:5:%d", i)
    n[i] = anode()
    n[i]:listen(a[i])
    print(a[i])
  end

  for i = 1, NODES do
    for j = 1, PEERS do
      paddr = ((i + j - 1) % NODES) + 1;
      peer_id = n[i]:add_peer(string.format("sim://150:1000:4:%d", paddr))
      print(i, j, paddr, peer_id)
      n[i]:connect(peer_id)
    end
  end
end

function tcp_test()
  N = 20
  M = 4
  START_PORT = 5000

  function addr(i)
    return "tcp://127.0.0.1:" .. tostring(START_PORT + i)
  end

  for i = 1, N do
    nodes[i] = anode()
    print(addr(i))
    nodes[i]:listen(addr(i))
  end

  for i = 1, N do
    for j = 1, M do
      a = (i + j - 1) % N + 1
      print(addr(i), "->", addr(a))
      peer_id = nodes[i]:add_peer(addr(a))
      nodes[i]:connect(peer_id)
    end
  end
end

function samir_test()
  a1 = "sim://5:5:1"
  a2 = "sim://5:13:2"

  n1 = anode()
  n2 = anode()

  n1:listen(a1)
  n2:listen(a2)

  p2 = n1:add_peer(addr)

  n1:connect(p2)

  -- n1:sendb(p2, "alabala", 1)
end
