#include "automaton/core/crypto/module.h"

#include "automaton/core/crypto/cryptopp/module.h"
#include "automaton/core/crypto/cryptopp/SHA256_cryptopp.h"
#include "automaton/core/crypto/ed25519_orlp/module.h"
#include "automaton/core/data/module.h"
#include "automaton/core/data/protobuf/module.h"
#include "automaton/core/io/module.h"
#include "automaton/core/log/module.h"
#include "automaton/core/network/module.h"
#include "automaton/core/script/registry.h"
#include "automaton/core/state/module.h"
#include "gtest/gtest.h"
#include "cryptlib.h"  // NOLINT
#include "hex.h"  // NOLINT

namespace automaton {
namespace core {

// Helper function to convert bytes to hex values
// Each byte is converted to 2 hex values, encoding the left and
// right 4 bits of each byte.
static std::string toHex(uint8_t * digest, size_t size) {
  CryptoPP::HexEncoder encoder;
  std::string output;
  encoder.Attach(new CryptoPP::StringSink(output));
  encoder.Put(digest, size);
  encoder.MessageEnd();
  return output;
}

class test_script : public ::testing::Test {
 protected:
  // You can define per-test set-up and tear-down logic as usual.
  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

common::obj* create_by_name(std::string name) {
  auto& r = script::registry::instance();
  auto& f = r.get_factory();
  auto id = f.get_schema_id(name);
  std::unique_ptr<data::msg> m = f.new_message_by_id(id);
  return r.create(*m.get());
}

TEST_F(test_script, module_registration) {
  auto& r = script::registry::instance();

  // import core module interfaces.
  r.import<crypto::module>();
  r.import<data::module>();
  r.import<io::module>();
  r.import<log::module>();
  r.import<network::module>();
  r.import<state::module>();

  // import core module implementations.
  r.import<crypto::cryptopp::module>();
  r.import<crypto::ed25519_orlp::module>();
  r.import<data::protobuf::module>();

  const char* blank = "";
  uint8_t digest[1024];

  std::string tests[][3] = {
    {"keccak256", "", "C5D2460186F7233C927E7DB2DCC703C0E500B653CA82273B7BFAD8045D85A470"},
    {"keccak256", "abc", "4E03657AEA45A94FC7D47BA826C8D667C0D1E6E33A64A036EC44F58FA12D6C45"},
    {"ripemd160", "", "9C1185A5C5E9FC54612808977EE8F548B2258D31"},
    {"sha256", "", "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855"},
    {"sha3", "", "A7FFC6F8BF1ED76651C14756A061D662F580FF4DE43B49FA82D80A4B80F8434A"},
    {"sha512", "",
        "CF83E1357EEFB8BDF1542850D66D8007D620E4050B5715DC83F4A921D36CE9CE"
        "47D0D13C5D85F2B0FF8318D2877EEC2F63B931BD47417A81A538327AF927DA3E"},
  };

  // Test instantiation and correctness of dynamically created hash_transformation objects.
  for (auto test : tests) {
    auto hash =
        dynamic_cast<crypto::hash_transformation*>(create_by_name("cryptopp.v0." + test[0]));

    hash->calculate_digest(
        reinterpret_cast<const uint8_t*>(test[1].c_str()), test[1].length(), &digest[0]);
    EXPECT_EQ(toHex(digest, hash->digest_size()), test[2]);
  }

  // Test instantiation of random object.
  auto random = dynamic_cast<crypto::secure_random*>(create_by_name("cryptopp.v0.random"));

  // Test instantiation of digital signatures object.
  auto secp256k1 =
     dynamic_cast<crypto::digital_signature*>(create_by_name("cryptopp.v0.secp256k1"));
  auto ed25519 =
     dynamic_cast<crypto::digital_signature*>(create_by_name("ed25519_orlp.v0.ed25519"));

  std::cout << r.to_string() << std::endl;
}

}  // namespace core
}  // namespace automaton
