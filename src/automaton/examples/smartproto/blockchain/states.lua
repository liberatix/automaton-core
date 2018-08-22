-- states
STATE = {}
STATE.HANDSHAKE = 1
STATE.BEHIND = 2
STATE.AHEAD = 3
STATE.DISCONECTED = 4
STATE.NOT_CONNECTED = 5
STATE.IN_CONSENSUS = 6

-- block state after validation
BLOCK = {}
BLOCK.VALID = 1
BLOCK.INVALID = 2
BLOCK.DUPLICATE = 3
BLOCK.NO_PARENT = 4

GENESIS_HASH = sha3("automaton")

peers = {}
blocks = {}
blockchain = {}
