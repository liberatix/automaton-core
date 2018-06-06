Node/ Protocol set up:
  * As a beginning everyone is a FULL node. Difficulty doesn't change. New block will consist of
  hash, hash of the previous block, height, and the address of the peer that found it. Everybody is
  mining and when a new hash is found (this could also be rpc call: process(*mine new block*)) it
  sends it to all other peers. Max_height and top_block_hash may be stored as node class members.
  * There is no disconnect and storage of state for use when connect again.
  * There is no peer discovery. A new peer joins the network by connecting to peers from a list.
  * A state storing <hash, block> is used by every peer to represent the blockchain.

Block validation and state synchronization:
 1. Check if hash satisfies difficulty. If no, ignore it.
 2. Check if hash is in the tree, if it is, ignore it.
 3. Check if prev hash is present in the chain/ tree.
  1. If prev hash is the top, add the new block to the top. See if there are orphan blocks that are
  children to this one. Do recursively.
  2. If prev hash is in the tree but not the top, split. See if there are orphan blocks that are
  children to this one. Do recursively.
  3. If prev hash is not in the tree, add the new block to orphan blocks set and request prev
  hash block
 4. Send the top of the chain.

```
syntax = "proto3";

message blocks_request {
/*
  Peer A send this when requesting blocks to show what is its top of the chain so if he is many
  blocks behind peer B will send him more blocks. If connects for the first time, this will be zero.
*/
  blob top_block_hash = 1;

  // Peer A sends the hash/es of the block/s he requests from B
  repeated blob block_hash_requested = 2;
}
message blocks_response {
  // Peer B sets the hash/es of the block/s he sends to A
  repeated blob block_hash_response = 1;
   // Peer B sends the whole block/s with hash/es block_hash_response to A
  repeated blob block = 2;
}

message data {
  blocks_request msg_type_1 = 1;
  blocks_response msg_type_2 = 2;
}

```
