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

```

### Get hash
```

```

### Get children
```

```


...
