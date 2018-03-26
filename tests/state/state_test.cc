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

  state_impl state;

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
  state_impl state;
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



/*
TEST(state_impl, set_multiple_times) {
  SHA256_cryptopp hash;
  state_impl s(&hash);
  s.set("a", "1");
  EXPECT_EQ(s.get("a"), "1");
  s.set("a", "");
  EXPECT_EQ(s.get("a"), "");
  s.commit_changes();
  EXPECT_EQ(s.get("a"), "");
}


TEST(state_impl, insert_and_delete_expect_blank) {
  state_impl s;
  s.set("a", "1");
  s.set("b", "2");
  s.set("c", "3");
  s.commit_changes();
  s.set("a", "");
  s.set("b", "");
  s.set("c", "");
  s.commit_changes();
  EXPECT_EQ(s.get(""), "");
  //EXPECT_EQ(s.get_node_hash(""), "");
}
*/

/*
TEST(state_impl, get_node_hash) {
  SHA256_cryptopp hash;
  state_impl s(&hash);
  EXPECT_EQ(s.get_node_hash(""), "");
}

TEST(state_impl, get_should_not_change_state) {
  SHA256_cryptopp hash;
  state_impl s(&hash);
  EXPECT_EQ(s.get("a"), "");
  EXPECT_EQ(s.get_node_hash(""), "");
}

TEST(state_impl, commit_changes) {
  SHA256_cryptopp hash;
  state_impl s(&hash);
  s.set("a", "1");
  s.set("b", "2");
  s.set("c", "3");
  s.commit_changes();
  EXPECT_EQ(s.get_node_hash(""), "O2\x4Je_2\xE8R\x8E\xDE\xA6M\xBF\xD1\x1C\xBA\x81\v\x87\x90\xE6\xE6\xE2=(\xAD:u\x98\a4"); // NOLINT
}

TEST(state_impl, discard_changes) {
  SHA256_cryptopp hash;
  state_impl s(&hash);
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
  s.commit_changes();
  s.set("a", "test");
  s.delete_node_tree("a");
  s.commit_changes();
  EXPECT_EQ(s.get_node_hash(""), "");
}
*/
