a1 = "tcp://127.0.0.1:5001"
a2 = "tcp://127.0.0.1:5002"

-- Google address
-- a1 = "69.172.200.235:80"

--[[
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
]]

n1 = anode()
n2 = anode()

n1:listen(a1)
n2:listen(a2)

p2 = n1:add_peer(a2)
n1:connect(p2)

-- p1 = n2:add_peer(a1)
-- n2:connect(p1)
