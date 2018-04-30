#ifndef AUTOMATON_CORE_EXAMPLES_SIMPLE_MINER_H__
#define AUTOMATON_CORE_EXAMPLES_SIMPLE_MINER_H__

#include <string>
#include <vector>
#include "crypto/hash_transformation.h"
#include "examples/crypto/simple_miner.h"

hash_transformation* _hash_transformation;
unsigned char* _nonce;
const int _nonce_lenght = 32;

void next_nonce() {
  int current = _nonce_lenght - 1;

  while (_nonce[current] == 255 && current > 0) {
    _nonce[current] = 0;
    current--;
  }

  _nonce[current]++;
}

bool is_valid_next_block_hash(unsigned char* hash, int required_leading_zeros) {
  int current = 0;
  while (required_leading_zeros - 8 >= 0) {
    if (hash[current] != 0) {
      return false;
    }
    current++;
    required_leading_zeros -= 8;
  }

  if (required_leading_zeros > 0) {
    return (hash[current] & ((1 << ( 8 - required_leading_zeros)) - 1))
           == hash[current];
  } else {
    return true;
  }
}

int simple_miner::get_nonce_lenght() {
  return _nonce_lenght;
}

simple_miner::simple_miner(hash_transformation* hash_transformation) {
  _hash_transformation = hash_transformation;
}

unsigned char* simple_miner::mine(unsigned char* block_hash,
    int block_hash_lenght,
    int required_leading_zeros) {
  if (required_leading_zeros > _nonce_lenght * 8) {
    // TODO(martin) log the invalid input
    return NULL;
  }

  int digest_size = _hash_transformation -> digest_size();
  unsigned char* next_block_hash = new unsigned char[digest_size];
  _nonce = new unsigned char[_nonce_lenght]();

  do {
    next_nonce();
    _hash_transformation -> restart();

    _hash_transformation -> update(block_hash, block_hash_lenght);
    _hash_transformation -> update(_nonce, _nonce_lenght);
    _hash_transformation -> final(next_block_hash);
  } while (!is_valid_next_block_hash(next_block_hash, required_leading_zeros));

  return _nonce;
}

#endif  // AUTOMATON_CORE_EXAMPLES_SIMPLE_MINER_H__
