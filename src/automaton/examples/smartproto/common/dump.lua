-- dump.lua

function dump_logs(path)
  if networks[path] == nil then
    print("NO SUCH PATH in NETWORKS " .. path)
    return
  end
  dump_connections_graph(path)
  for i in pairs(networks[path]["nodes"]) do
    networks[path]["nodes"][i]:dump_logs(string.format("%slogs/N%03d-%s.html", path, i, names[i]))
  end
  collect_states(networks[path]["nodes"])
end

function dump_connections_graph(path)
  if networks[path]["graph"] == nil then
    return
  end
  file = io.open(path .. "logs/connections_graph.html", "w+")
  file:write(create_graph_html(networks[path]["graph"]))
  file:close()
end
