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
  NODES = 10
  PEERS = 16

  a={}

  for i = 1, NODES do
    a[i] = string.format("sim://5:5:%d", i)
    nodes[i] = anode()
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

START_PORT = 5000

function tcp_addr(i)
  return "tcp://127.0.0.1:" .. tostring(START_PORT + i)
end

function tcp_test()
  N = 20
  M = 4

  for i = 1, N do
    nodes[i] = anode()
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

p = n[5000]:peers()
print("Connected peers " .. #p)
]]

a1 = "tcp://127.0.0.1:5001"
a2 = "tcp://127.0.0.1:5002"
n1 = anode()
n2 = anode()

  n1:listen(a1)
  n2:listen(a2)



p2 = n1:add_peer(a2)

n1:connect(p2)
function dump_logs()
  n1:dump_logs(string.format("logs/N%d.html", 1))
  n2:dump_logs(string.format("logs/N%d.html", 2))
end
-- n1:sendb(p2, "alabala", 1)
