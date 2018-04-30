#ifndef AUTOMATON_CORE_EXAMPLES_MINER_DEMO_H__
#define AUTOMATON_CORE_EXAMPLES_MINER_DEMO_H__

#include <string>
#include "crypto/hash_transformation.h"
#include "crypto/SHA256_cryptopp.h"
#include "examples/crypto/simple_miner.h"

char hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
'a', 'b', 'c', 'd', 'e', 'f' };

std::string hex_string(unsigned char *data, int len) {
    std::string s(len * 2, ' ');
    for (int i = 0; i < len; ++i) {
        s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
        s[2 * i + 1] = hexmap[data[i] & 0x0F];
    }
    return s;
}

int main() {
    SHA256_cryptopp::register_self();
    hash_transformation* hash_transformation
        = hash_transformation::create("SHA256");
    simple_miner miner = simple_miner(hash_transformation);

    std::string demo_hash("some block hash");
    unsigned char* block_hash = (unsigned char*)demo_hash.c_str();

    unsigned char* nonce = miner.mine(block_hash, demo_hash.length(), 12);

    int digest_size = hash_transformation -> digest_size();
    unsigned char* next_block_hash = new unsigned char[digest_size];

    hash_transformation->update(block_hash, demo_hash.length());
    hash_transformation->update(nonce, miner.get_nonce_lenght());
    hash_transformation->final(next_block_hash);

    std::cout << "     nonce: "
        << hex_string(nonce, miner.get_nonce_lenght()) << std::endl;
    std::cout << "block hash: "
        << block_hash << std::endl;
    std::cout << "mined hash: "
        << hex_string(next_block_hash, digest_size) << std::endl;

    return 0;
}

#endif  // AUTOMATON_CORE_EXAMPLES_MINER_DEMO_H__
