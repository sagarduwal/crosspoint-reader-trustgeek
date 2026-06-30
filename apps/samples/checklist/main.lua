-- Checklist: up to 8 persistent todo items.

local MAX_ITEMS = 8

local defaults = {
  "Read one chapter",
  "Review notes",
  "Return library book",
}

local items = {}
local selected = 1

local function serialize_items()
  local lines = {}
  for i, item in ipairs(items) do
    lines[i] = (item.done and "1" or "0") .. "|" .. item.text
  end
  return table.concat(lines, "\n")
end

local function deserialize_items(data)
  local parsed = {}
  for line in data:gmatch("[^\r\n]+") do
    local done_flag, text = line:match("^(%d)|(.+)$")
    if text then
      parsed[#parsed + 1] = { text = text, done = done_flag == "1" }
    end
  end
  return parsed
end

local function load_items()
  if cp.fs.exists("items.txt") then
    local data = cp.fs.read("items.txt")
    if data then
      local parsed = deserialize_items(data)
      if #parsed > 0 then
        items = parsed
        return
      end
    end
  end
  items = {}
  for _, text in ipairs(defaults) do
    items[#items + 1] = { text = text, done = false }
  end
end

local function save_items()
  cp.fs.write("items.txt", serialize_items())
end

local function draw()
  cp.display.clear()
  cp.display.center(40, "Checklist")
  local y = 70
  for i, item in ipairs(items) do
    if i > MAX_ITEMS then
      break
    end
    local mark = item.done and "[x]" or "[ ]"
    local prefix = (i == selected) and ">" or " "
    cp.display.center(y, prefix .. mark .. " " .. item.text)
    y = y + 18
  end
  cp.display.center(400, "Up/Down move  Confirm toggle")
  cp.display.center(418, "Right delete  Back exit")
  cp.display.refresh()
end

load_items()

while true do
  draw()
  local btn = cp.input.wait()
  if btn == "back" then
    cp.sys.exit()
  elseif btn == "up" then
    if #items > 0 then
      selected = ((selected - 2) % #items) + 1
    end
  elseif btn == "down" then
    if #items > 0 then
      selected = (selected % #items) + 1
    end
  elseif btn == "confirm" then
    if items[selected] then
      items[selected].done = not items[selected].done
      save_items()
    end
  elseif btn == "right" then
    if items[selected] then
      table.remove(items, selected)
      if #items == 0 then
        selected = 1
      elseif selected > #items then
        selected = #items
      end
      save_items()
    end
  end
end
