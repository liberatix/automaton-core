#include "automaton/core/script/lua/lua_script_engine.h"

#include "automaton/core/crypto/cryptopp/Keccak_256_cryptopp.h"
#include "automaton/core/crypto/cryptopp/RIPEMD160_cryptopp.h"
#include "automaton/core/crypto/cryptopp/secure_random_cryptopp.h"
#include "automaton/core/crypto/cryptopp/SHA256_cryptopp.h"
#include "automaton/core/crypto/cryptopp/SHA3_256_cryptopp.h"
#include "automaton/core/crypto/cryptopp/SHA512_cryptopp.h"
#include "automaton/core/crypto/ed25519_orlp/ed25519_orlp.h"

using automaton::core::crypto::cryptopp::Keccak_256_cryptopp;
using automaton::core::crypto::cryptopp::RIPEMD160_cryptopp;
using automaton::core::crypto::cryptopp::secure_random_cryptopp;
using automaton::core::crypto::cryptopp::SHA256_cryptopp;
using automaton::core::crypto::cryptopp::SHA512_cryptopp;
using automaton::core::crypto::cryptopp::SHA3_256_cryptopp;

namespace automaton {
namespace core {
namespace script {
namespace lua {

void lua_script_engine::bind_crypto() {
  ripemd160.reset(new RIPEMD160_cryptopp());
  sha512.reset(new SHA512_cryptopp());
  sha256.reset(new SHA256_cryptopp());
  sha3.reset(new SHA3_256_cryptopp());
  keccak256.reset(new Keccak_256_cryptopp());

  lua.set_function("ripemd160", [&](const std::string& s) -> std::string {
    uint8_t digest[20];
    ripemd160->calculate_digest(reinterpret_cast<const uint8_t*>(s.data()), s.size(), digest);
    return std::string((char*)digest, 20); // NOLINT
  });

  lua.set_function("sha512", [&](const std::string& s) -> std::string {
    uint8_t digest[64];
    sha512->calculate_digest(reinterpret_cast<const uint8_t*>(s.data()), s.size(), digest);
    return std::string((char*)digest, 64); // NOLINT
  });

  lua.set_function("sha256", [&](const std::string& s) -> std::string {
    uint8_t digest[32];
    sha256->calculate_digest(reinterpret_cast<const uint8_t*>(s.data()), s.size(), digest);
    return std::string((char*)digest, 32); // NOLINT
  });

  lua.set_function("sha3", [&](const std::string& s) -> std::string {
    uint8_t digest[32];
    sha3->calculate_digest(reinterpret_cast<const uint8_t*>(s.data()), s.size(), digest);
    return std::string((char*)digest, 32); // NOLINT
  });

  lua.set_function("keccak256", [&](const std::string& s) -> std::string {
    uint8_t digest[32];
    keccak256->calculate_digest(reinterpret_cast<const uint8_t*>(s.data()), s.size(), digest);
    return std::string((char*)digest, 32); // NOLINT
  });
}

}  // namespace lua
}  // namespace script
}  // namespace core
}  // namespace automaton
