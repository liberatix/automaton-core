print("================== SIMULATION ==================")

NODES = 2
PEERS = 1

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

function dump_logs()
  for i = 1, NODES do
    n[i]:dump_logs(string.format("logs/N%d.html", i))
  end
end

--[[
a1 = "sim://5:5:1"
a2 = "sim://5:13:2"

addr = "sim://150:1000:4:1"
-- Google address
-- a1 = "69.172.200.235:80"

n = {}
N = 50
M = 8

function addr(i)
  return "tcp://127.0.0.1:" .. tostring(i)
end

for i = 5000,5000 + N do
  n[i] = anode()
  print(addr(i))
  n[i]:listen(addr(i))
end

for i = 5000,5000 + N do
  for j = 1, M do
    a = 5000 + ((i + j) - 5000) % N
    print(i, a)
    n[i]:connect(n[i]:add_peer(addr(a)))
  end
end

p = n[5000]:peers()
print("Connected peers " .. #p)

n1 = anode()
n2 = anode()

n1:listen(a1)
n2:listen(a2)

p2 = n1:add_peer(addr)

n1:connect(p2)

-- n1:sendb(p2, "alabala", 1)
]]
