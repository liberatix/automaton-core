-- GRAPH VISUALIZATION OF BLOCK TREE

function debug_html()
  local n = {}
  local e = {}

  local bb = {}
  for i = 1, #blockchain do
    table.insert(bb, tostring(i) .. ": " .. hex(blockchain[i]))
  end

  -- GENESIS_HASH
  local s
  GH = hex(GENESIS_HASH):sub(3,8)
  s = string.format("{id: '%s', label: 'GENESIS', color: '#D2B4DE', level: %d}", GH, 0)
  table.insert(n, s)

  local clr
  for k,v in pairs(blocks) do
    to = hex(k):sub(3,8)
    from = hex(v.prev_hash):sub(3,8)
    -- check if this is in current blockchain
    if k == blockchain[v.height] then
      clr = "'#ABEBC6'"
    else
      clr = "'#F5CBA7'"
    end
    title = "mined by " .. v.miner
    s = string.format("{id: '%s', label: '%s', color: %s, level: %d, title: '%s'}",
      to, to, clr, v.height, title)
    table.insert(n, s)
    s = string.format("{from: '%s', to: '%s', arrows:'to'}", from, to)
    table.insert(e, s)
  end

  local html =
[[
<div id="mynetwork"></div>

<script type="text/javascript">
  // create an array with nodes
  var nodes = new vis.DataSet([
]]
..

  table.concat(n, ",\n")
  --[[
    {id: "a1", label: 'Node 1'},
    {id: "a2", label: 'Node 2'},
    {id: "a3", label: 'Node 3'},
    {id: "a4", label: 'Node 4'},
    {id: "a5", label: 'Node 5'},
    {id: "a6", label: 'Node 6'},
    {id: "a7", label: 'Node 7'},
    {id: "a8", label: 'Node 8'}
  ]]

..
[[
  ]);
  // create an array with edges
  var edges = new vis.DataSet([
]]  
..

  table.concat(e, ",\n")
--[[
    {from: "a1", to: "a8", arrows:'to', dashes:true},
    {from: "a1", to: "a3", arrows:'to'},
    {from: "a1", to: "a2", arrows:'to, from'},
    {from: "a2", to: "a4", arrows:'to, middle'},
    {from: "a2", to: "a5", arrows:'to, middle, from'},
    {from: "a5", to: "a6", arrows:{to:{scaleFactor:2}}},
    {from: "a6", to: "a7", arrows:{middle:{scaleFactor:0.5},from:true}}

]]

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
        levelSeparation: 80,
        nodeSpacing: 50
      }
    },
    physics: false
  };
  var network = new vis.Network(container, data, options);
</script>
]]

  return html;
end