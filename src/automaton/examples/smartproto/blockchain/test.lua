--[[
ITERATIONS = 100000

x = 5
print(hex(sha3(abc)))

blocks = {}

m = Block()

m.miner = "test"

print(hex(m:serialize()))

t = os.clock()
for i = 1, ITERATIONS do
  m.miner = i
--  blocks[#blocks + 1] = m:serialize()
end
t = os.clock() - t
print(string.format("create/set_name/serialize [%.3f M/s]", ITERATIONS / t / 1000000))

t = os.clock()
for i = 1, ITERATIONS do
  m:set_blob(1, i)
--  blocks[#blocks + 1] = m:serialize()
end
t = os.clock() - t
print(string.format("create/set_id/serialize [%.3f M/s]", ITERATIONS / t / 1000000))

print(m.miner)
print(#blocks)

]]
