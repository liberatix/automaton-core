# Smart Protocols

```protobuf

message Tx {
  
}

message BlockHeader {
  bytes prev_block_hash = 1;
  bytes nonce = 2;
  repeated bytes txs_hash = 3;
}

message Block {
  BlockHeader header;
}

```

```js

protocol SimpleBlockchain {
  map(bytes => Block) blocks;
  map(bytes => Tx) txs;

  Tx[] pending;
}

```
