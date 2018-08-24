-- show_states.lua

function collect_states()
  local states = {}
  for i in pairs(nodes) do
    local name = names[i]
    local blocks_string = _G[names[i]].get_stats();
    local blocks = {}
    for block in string.gmatch(blocks_string, "%S+") do
      table.insert(blocks, block)
    end
    table.insert(states, {n = name, b = blocks})
  end
  dump_node_states(states)
end

GENESIS_HASH = sha3("automaton")

function fmt_node(hash, names)
  local fmt = "{id:'%s', label:'%s\\n%d nodes'}"
  return string.format(fmt, hash, hash:sub(-8), #names)
end

function dump_node_states(n)
  local file = io.open("logs/node_states.html", "w+")

  local node_blocks = {}
  local edges = {}
  local prev_block = {}
  local added = {}
  local block_names = {}

  local gen_hash = hex(GENESIS_HASH)
  table.insert(node_blocks, fmt_node(gen_hash, {}))

  -- gather blocks that each node has
  for s1,state in pairs(n) do
    -- only show the last N blocks from each node
    local f = false
    while #state.b > 20 do
      table.remove(state.b, 1)
      f = true
    end
    if f then table.insert(state.b, 1, "...") end

    for k,v in pairs(state.b) do
      if block_names[v] == nil then
        block_names[v] = {}
      end
      table.insert(block_names[v], state.n)
    end
  end

  for s1,state in pairs(n) do
    for k,v in pairs(state.b) do
      if added[v] == nil then
        added[v] = true
        table.insert(node_blocks, fmt_node(v, block_names[v]))
        if k > 1 then
          prev_block[v] = state.b[k-1]
        else
          prev_block[v] = hex(GENESIS_HASH)
        end
      end
    end
  end

  for k,v in pairs(prev_block) do
    e = string.format(
      "{from: '%s', to: '%s', arrows:'to'}",
      v, k
    )
    table.insert(edges, e)
  end

  html = [[
<html>
<head>

<script type="text/javascript" src="../js/vis.min.js"></script>
<link href="../js/vis.min.css" rel="stylesheet" type="text/css" />

<style type="text/css">
  body {
    font-family: 'Play';
  }
  #mynetwork {
    width: 1000px;
    height: 600px;
    border: 1px solid lightgray;
  }

  pre {
    border: 1px solid black;
    padding: 8px;
    overflow:auto;
    font-size: 16px;
    font-family: 'Inconsolata', monospace;
  }

</style>
</head>
<body>
<h1>
Aggregated blocks from all nodes
</h1>
<div id="mynetwork"></div>

<script type="text/javascript">
  // create an array with nodes
  var nodes = new vis.DataSet([
  ]]
  ..
    table.concat(node_blocks, ",\n")
  ..
  [[
    ]);
    // create an array with edges
    var edges = new vis.DataSet([
  ]]
  ..
    table.concat(edges, ",\n")
  ..
  [[
    ]);

  // create a network
  var container = document.getElementById('mynetwork');
  var data = {
    nodes: nodes,
    edges: edges
  };
  var options = {
    edges: {
      smooth: {
        type: 'cubicBezier',
        forceDirection: 'horizontal',
        roundness: 0.4
      }
    },
    layout: {
      hierarchical: {
        direction: "LR",
        levelSeparation: 120,
        nodeSpacing: 100
      }
    },
    physics: false
  };

  var network = new vis.Network(container, data, options);
</script>
</body></html>
  ]]

  file:write(html)
  file:close()
end