# Automaton data module reference

## state interface

**state.h**

```cpp
class state {
 public:
  // Get the value at given path. Empty string if no value is set or
  // there is no node at the given path
  string get(string key) = 0;

  // Set the value at a given path
  void set(string key, string value) = 0;

  // Get the hash of a node at the given path. Empty string if no value is
  // set or there is no node at the given path
  string get_node_hash(string path) = 0;

  // Get the children as string
  vector<string> get_node_children(string path) = 0;

  // delete subtree with root the node at the given path
  void delete_node_tree(string path) = 0;

  // finalizes the changes made by set
  void commit_changes() = 0;

  // discards the changes made by set;
  void discard_changes() = 0;
}
```

## Examples:

### Get value
```cpp
get("a");
// -> ""
set("abc", "123");
get("ab")
// -> ""
get("abc", "123");
// -> "123"
```

### Get hash
```cpp
get_node_hash("");
// -> ""
set("abc", "123");
get_node_hash("");
// -> "hash of root"
get_node_hash("a");
// -> "hash of node a"
get_node_hash("abc");
// -> "hash of node abc"
```

### Get children
```cpp
set("abc", "1");
set("acc", "2");
set("ccc", "3");
vector<string> children = get_node_children("");
// children contains:
// ->{"a", "c"}
children = get_node_children("a");
// children contains:
// ->{"b", "c"}
children = get_node_children("ab");
// children contains:
// ->{"c"}
children = get_node_children("ccc");
// children contains:
// ->{""}
```

### delete_node_tree
```cpp
set("abc", "1");
set("acc", "2");
set("acd", "3");
delete_node_tree("ac");
// -> element acc, acd and ac are deleted;
delete_node_tree("abc");
// -> element abb gets deleted, the tree contains no nodes.
```


### commit_changes and discard_changes
```cpp
set("ab", "1");
get("ab");
// -> "1"
commit_changes();
set("cd", "2");
get("ab");
// -> "1"
get("cd")
// -> "2"
discard_changes();
get("ab");
// -> "1"
get("cd")
// -> ""
// Changed made after commit_changes() was called are
// discarded when discard_changes is called
delete_node_tree("ab");
get("ab");
// -> ""
discard_changes();
get("ab");
// -> "1"
```
