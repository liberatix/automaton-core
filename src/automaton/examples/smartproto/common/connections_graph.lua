-- connections_graph.lua

function create_graph_html()
  local html = [[
<html>
<head>

<script type="text/javascript" src="../js/vis.min.js"></script>
<link href="../js/vis.min.css" rel="stylesheet" type="text/css" />

<style type="text/css">
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
<br/>
<hr />
<div id="mynetwork"></div>

<script type="text/javascript">
  // create an array with nodes
  var nodes = new vis.DataSet([
  ]]
  ..
    table.concat(connections_graph_nodes, ",\n")

  ..
  [[
    ]);
    // create an array with edges
    var edges = new vis.DataSet([
  ]]
  ..
    table.concat(connections_graph_edges, ",\n")
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
    physics: {
      stabilization: {
        iterations: 500
      }
    }
  };
  var network = new vis.Network(container, data, options);
</script>
</body></html>
  ]]
  return html
end
