# Smart Protocols

```protobuf

message RegisterValidatorSlot {
  bytes signature = 1;
  bytes public_key = 2;
}

```

```js

REGISTER_VALIDATOR_SLOT_MSG = "Register Validator Slot";

function init() {
  state.slots_mask = 0x3FF;
  ecdsa = dsig.create("secp256k1")
}

function onRegisterValidatorSlot(peer, msg) {
  if (!ecdsa.verify(msg.signature,
                    msg.public_key,
                    REGISTER_VALIDATOR_SLOT_MSG)) {
    // Punish peers who are wasting our resources.
    peer.punish();
    return;
  }

  slot = msg.public_key & state.slots_mask;
  if (msg.validator_slots[slot] > msg.public_key) {
    msg.validator_slots[slot] = msg.public_key;
  }
}

function onMessage(peer, msg) {
  if (msg.type == 'RegisterValidatorSlot') {
    onRegisterValidatorSlot(peer, msg);
  }
}

```
