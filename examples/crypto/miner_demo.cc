#ifndef AUTOMATON_CORE_EXAMPLES_MINER_DEMO_H__
#define AUTOMATON_CORE_EXAMPLES_MINER_DEMO_H__

#include <string>
#include "crypto/hash_transformation.h"
#include "crypto/cryptopp/SHA256_cryptopp.h"
#include "examples/crypto/basic_hash_miner.h"

using automaton::core::crypto::hash_transformation;
using automaton::core::crypto::SHA256_cryptopp;
using automaton::core::examples::basic_hash_miner;

const char DEMO_HASH[] = "some block hash";
const int MINER_PRECISION_BITS = 14;

std::string to_hex_string(unsigned char *data, int len) {
    // TODO(martin) remove this method when there is
    //              a proper library implemented
    const char hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                            'a', 'b', 'c', 'd', 'e', 'f' };

    std::string s(len * 2, ' ');
    for (int i = 0; i < len; ++i) {
        s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
        s[2 * i + 1] = hexmap[data[i] & 0x0F];
    }

    return s;
}

int main() {
    unsigned char* block_hash = (unsigned char*)DEMO_HASH;

    SHA256_cryptopp::register_self();
    auto hash_transformation = hash_transformation::create("SHA256");
    basic_hash_miner miner(hash_transformation);

    unsigned char* nonce = miner.mine(
      block_hash,
      std::strlen(DEMO_HASH),
      MINER_PRECISION_BITS);

    int digest_size = hash_transformation -> digest_size();
    unsigned char* next_block_hash = new unsigned char[digest_size];

    hash_transformation->update(block_hash, std::strlen(DEMO_HASH));
    hash_transformation->update(nonce, miner.get_nonce_lenght());
    hash_transformation->final(next_block_hash);

    std::cout << "     nonce: "
        << to_hex_string(nonce, miner.get_nonce_lenght()) << std::endl;
    std::cout << "block hash: "
        << block_hash << std::endl;
    std::cout << "mined hash: "
        << to_hex_string(next_block_hash, digest_size) << std::endl;

    return 0;
}

#endif  // AUTOMATON_CORE_EXAMPLES_MINER_DEMO_H__
