function update(time)
  if table_length(validators) == TOTAL_VALIDATORS then
    update_sate(time)
  end
end

function sent(peer_id, msg_id, success)
end

function on_RegisterValidators(peer_id, msg)
  log("on_RegisterValidators", "call by peer: " .. tostring(peer_id))
  local new_validator = false
  for k,v in pairs(msg.public_key) do
    if validators[v] == nil then
      log("new validator from ", nodeid .. " RECEIVED " .. hex(v))
      validators[v] = 1
      new_validator = true
    end
  end
  if new_validator then
    gossip(peer_id, validators_msg())
    if table_length(validators) == TOTAL_VALIDATORS then
      for k, v in pairs(validators) do
        table.insert(validators_sorted, k)
      end
      table.sort(validators_sorted)
      --log("total_validators ", validators_sorted)
      for i,n in ipairs(validators_sorted) do
        log("validators", tostring(i) .. ": " .. hex(n))
        if public_key == n then
          our_slot = i
          log("validators", tostring(i) .. " IS OUR SLOT!")
        end
      end
    end
  end
end

function on_StateTransition(peer_id, state_transition)
end

function on_CreateReservation(peer_id, reservation)
end

function on_CancelReservation(peer_id, cancellation)
end

function debug_html()
  local html = ""
  for i = 1, ROOM_COUNT do
    html = html .. get_room_html(i)
  end
  html = html .. [[
  <div id="noteID", class="note">Tooltip text</div>
  <br />
  <style>
  .busy {
  	margin: 10px;
      padding: 5px;
      background: #d5c7d5;
      color: black
  }
  .selected {
  	margin: 10px;
      padding: 5px;
      background: #58b7bf;
      color: black
  }
  table {
  	width: 300px;
      height: 300px;
      font-family: Verdana, sans-serif;
      font-size: 18px;
      background: #edf6f6;
      align: left;
      text-align: center;
      border: "0";
      margin: 0;
      cursor: pointer;
  }
  caption {
  	align: "center";
      text-align: center;
      font-size: 25px;
      border: "0";
      margin: 0;
      background: #7dc2c8;
      cursor: default;
  }
  td {
  	margin: 10px;
      padding: 5px;
  }
  .note {
  	visibility: hidden;
      font-family: Verdana, sans-serif;
      font-size: 18px;
      background: #edf6f6;
      align: right;
      text-align: center;
      border: "0";
      margin: 0;
      position: absolute;
      top: 150px;
      left: 35%;
  }
  </style>
  <script>
  var tables = document.getElementsByClassName("calendar");
  for (var k = 0; k < tables.length; k++) {
    var table = tables[k]
  	for (var i = 0; i < table.rows.length; i++) {
          for (var j = 0; j < table.rows[i].cells.length; j++) {
  			table.rows[i].cells[j].onmouseover = function () {
  				on_selection(this, k, true);
  			};
              table.rows[i].cells[j].onmouseleave = function () {
  				on_selection(this, k, false);
  			};
          }
  	}
  }

  function on_selection(table_cell, table_number, over) {
  	var current = document.getElementsByClassName("selected");
      if (current.length > 0) {
        current[0].className = current[0].className.replace(" selected", "");
      }
      var note = document.getElementById("noteID");
      if(over) {
      	table_cell.className += " selected";
          note.innerHTML = table_cell.innerHTML;
      	note.style = "visibility:visible";
      }
      else {
      	note.style = "visibility:hidden";
      }
  }
  </script>
  ]]
  return html
end

function get_room_html(room_number)
  local table_html = [[
  <table class="calendar">
    <caption>ROOM ]] .. tostring(room_number) .. [[</caption>
  ]]
  for i = 1, DAYS do
    if math.mod(i, 5) == 0 then
      table_html = table_html .. "</tr>"
    end
    if rooms[room_number][i] then
      table_html = table_html .. "<td class=\"busy\" id=" .. rooms[room_number][i] .. ">" .. tostring(i) .. "</td>"
    else
      table_html = table_html .. "<td>" .. tostring(i) .. "</td>"
    end
    if math.mod(i, 5) == 0 then
      table_html = table_html .. "</tr>"
    end
  end
  table_html = table_html .. "</table>"
  return table_html
end
