#include <string>
#include <vector>
#include <utility>
#include "gtest/gtest.h"
#include "state/state_impl.h"
#include "crypto/SHA256_cryptopp.h"


TEST(state_impl, set_and_get) {
  std::vector<std::pair<std::string, std::string> > tests;
  tests.push_back(std::make_pair("test", "1"));
  tests.push_back(std::make_pair("tester", "2"));
  tests.push_back(std::make_pair("ala", "ala"));
  tests.push_back(std::make_pair("alabala", "alabala"));
  tests.push_back(std::make_pair("testing", "3"));
  tests.push_back(std::make_pair("telting", "3.1"));
  tests.push_back(std::make_pair("travel", "4"));
  tests.push_back(std::make_pair("tramway", "5"));
  tests.push_back(std::make_pair("tram", "6"));
  tests.push_back(std::make_pair("tramva", "7"));

  SHA256_cryptopp::register_self();
  hash_transformation* hasher;
  hasher = hash_transformation::create("SHA256");
  state_impl state(hasher);

  // For each node added, check if the previous nodes are still correct
  for (unsigned int i = 0; i < tests.size(); i++) {
    state.set(tests[i].first, tests[i].second);
      for (unsigned int j = 0; j <= i; j++) {
        EXPECT_EQ(state.get(tests[j].first), tests[j].second);
      }
  }
}

TEST(state_impl, set_delete_and_get) {
  std::vector<std::pair<std::string, std::string> > tests;
  tests.push_back(std::make_pair("test", "1"));
  tests.push_back(std::make_pair("tester", "2"));
  tests.push_back(std::make_pair("ala", "ala"));
  tests.push_back(std::make_pair("alabala", "alabala"));
  tests.push_back(std::make_pair("testing", "3"));
  tests.push_back(std::make_pair("telting", "3.1"));
  tests.push_back(std::make_pair("travel", "4"));
  tests.push_back(std::make_pair("tramway", "5"));
  tests.push_back(std::make_pair("tram", "6"));
  tests.push_back(std::make_pair("tramva", "7"));

  SHA256_cryptopp::register_self();
  hash_transformation* hasher;
  hasher = hash_transformation::create("SHA256");
  state_impl state(hasher);
  // add all nodes
  for (unsigned int i = 0; i < tests.size(); i++) {
    state.set(tests[i].first, tests[i].second);
  }
  // delete one and check if remaining nodes are currect
  for (unsigned int i = 0; i < tests.size(); i++) {
    state.erase(tests[i].first);
    for (unsigned int j = i+1; j < tests.size(); j++) {
      EXPECT_EQ(state.get(tests[j].first), tests[j].second);
    }
  }
}

static std::string tohex(std::string s) {
  std::stringstream ss;
  for (unsigned int i = 0; i < s.size(); i++) {
    ss << std::hex << std::uppercase << std::setw(2) <<
        std::setfill('0') << (static_cast<int>(s[i]) & 0xff);
  }
  return ss.str();
}

std::string hash_key(int i) {
  uint8_t digest32[32];
  hash_transformation* hasher;
  hasher = hash_transformation::create("SHA256");
  std::string data = std::to_string(i);
  hasher->calculate_digest((const uint8_t*) data.c_str(), data.length(),
      digest32);
  return std::string(reinterpret_cast<char*>(digest32), 16 + i % 16);
}

TEST(state_impl, node_hash_add_erase) {
  std::stack<std::string> root_hashes;
  std::stack<std::string> keys;
  int32_t key_count = 10000;

  SHA256_cryptopp::register_self();
  hash_transformation* hasher;
  hasher = hash_transformation::create("SHA256");
  state_impl state(hasher);

  // Add keys/values to the state and add the root hash into a stack.
  for (int32_t i = 0; i < key_count; ++i) {
    root_hashes.push(state.get_node_hash(""));
    std::string key = hash_key(i);
    std::string data = std::to_string(i);
    keys.push(key);

    state.set(keys.top(), data);
    EXPECT_EQ(data, state.get(keys.top()));

    if (i % 1000) {
      continue;
    }
    // Integrity check for all prior key/values.
    std::cout << i << std::endl;
    for (int32_t j = 0; j <= i; j++) {
      std::string data = std::to_string(j);
      std::string key = hash_key(j);

      if (data != state.get(key)) {
        std::cout << "Setting " << i << " fails at " << j << std::endl;
        std::cout << "Setting key " << tohex(keys.top())
          << " fails " << tohex(key) << std::endl;
        throw "!!!";
      }
    }
  }

  // Erase the keys in reverse order and check if root hash is the saved one
  // for the same trie state

  for (int32_t i = 0; i < key_count; i++) {
    state.erase(keys.top());

    // Integrity check for all prior key/values.
    if (i % 1000 == 0) {
      std::cout << i << std::endl;
      for (int32_t j = 0; j < key_count - i - 1; j++) {
        std::string data = std::to_string(j);
        std::string key = hash_key(j);

        if (data != state.get(key)) {
          std::cout << "Deleting " << (key_count - i) << " fails at "
              << j << std::endl;
          std::cout << "Deleting key " << tohex(keys.top())
            << " fails " << tohex(key) << std::endl;
          throw std::domain_error("!!!");
        }
      }
    }

    keys.pop();

    if (i % 1000 == 0) {
      std::cout << "Passed " << i << " deletions in reverse order.\n";
    }
    EXPECT_EQ(state.get_node_hash(""), root_hashes.top());
    if (state.get_node_hash("") != root_hashes.top()) {
      throw std::domain_error("BAD " + std::to_string(i));
    }
    root_hashes.pop();
  }
}


TEST(state_impl, insert_and_delete_expect_blank) {

  SHA256_cryptopp::register_self();
  hash_transformation* hasher;
  hasher = hash_transformation::create("SHA256");
  state_impl state(hasher);

  state.set("a", "1");
  state.set("b", "2");
  state.set("c", "3");
  state.commit_changes();
  state.set("a", "");
  state.set("b", "");
  state.set("c", "");
  state.commit_changes();
  EXPECT_EQ(state.get(""), "");
}


TEST(state_impl, get_node_hash) {
  SHA256_cryptopp::register_self();
  hash_transformation* hasher;
  hasher = hash_transformation::create("SHA256");
  state_impl s(hasher);
  EXPECT_EQ(s.get_node_hash(""), "");
}

TEST(state_impl, commit_changes) {
  SHA256_cryptopp::register_self();
  hash_transformation* hasher;
  hasher = hash_transformation::create("SHA256");
  state_impl s(hasher);
  s.set("a", "1");
  s.set("b", "2");
  s.set("c", "3");
  std::string root_hash = s.get_node_hash("");
  s.commit_changes();
  EXPECT_EQ(s.get_node_hash(""), root_hash);
}

TEST(state_impl, discard_changes) {
  SHA256_cryptopp::register_self();
  hash_transformation* hasher;
  hasher = hash_transformation::create("SHA256");
  state_impl s(hasher);
  s.set("a", "1");
  s.set("b", "2");
  s.set("c", "3");
  s.discard_changes();
  EXPECT_EQ(s.get_node_hash(""), "");
}


TEST(state_impl, delete_node_tree) {
  SHA256_cryptopp hash;
  state_impl s(&hash);
  s.set("aa", "1");
  s.set("aaa", "2");
  s.set("abc", "3");
  s.set("a2z", "4");
  s.set("a", "test");
  s.delete_node_tree("a");
  EXPECT_EQ(s.get_node_hash(""), "");
}
