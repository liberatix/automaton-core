a1 = "127.0.0.1:5001"
a2 = "127.0.0.1:5002"

-- Google address
-- a1 = "69.172.200.235:80"

n1 = anode()
n2 = anode()

n1:add_peer(a1)
n1:add_peer(a2)

n1:listen(a1)
n2:listen(a2)

n1:connect(a2)

n2:add_peer(a1)
n2:connect(a1)

m1 = n1:new_msg(0)
m2 = n1:new_msg(2)
m2.block = m1

m = n1:new_msg(3)
m.f1 = 1
m.f1 = 2
m.f1 = 3
