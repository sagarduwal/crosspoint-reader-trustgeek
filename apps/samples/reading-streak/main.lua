-- Reading Streak: daily reading habit tracker.

local DAY_LABELS = { "M", "T", "W", "T", "F", "S", "S" }
local DATA_FILE = "streak.txt"

local streak = 0
local last_day = -1
local week_bits = 0
local view_offset = 0

local function day_index()
  return math.floor(cp.sys.millis() / 86400000)
end

local function serialize()
  return string.format("streak=%d\nlast_day=%d\nweek_bits=%d\n", streak, last_day, week_bits)
end

local function deserialize(data)
  local s = data:match("streak=(%d+)")
  local d = data:match("last_day=(%-?%d+)")
  local b = data:match("week_bits=(%d+)")
  if s then streak = tonumber(s) end
  if d then last_day = tonumber(d) end
  if b then week_bits = tonumber(b) end
end

local function load_state()
  if cp.fs.exists(DATA_FILE) then
    local data = cp.fs.read(DATA_FILE)
    if data then
      deserialize(data)
    end
  end
end

local function save_state()
  cp.fs.write(DATA_FILE, serialize())
end

local function rollover_if_needed()
  local today = day_index()
  if last_day < 0 then
    return
  end
  local gap = today - last_day
  if gap == 0 then
    return
  end
  if gap == 1 then
    week_bits = (week_bits * 2) % 128
    return
  end
  streak = 0
  week_bits = 0
end

local function mark_today()
  local today = day_index()
  rollover_if_needed()
  if last_day == today then
    return
  end
  if last_day >= 0 and today - last_day == 1 then
    streak = streak + 1
  else
    streak = 1
  end
  last_day = today
  week_bits = week_bits + 1
  save_state()
end

local function today_marked()
  return last_day == day_index()
end

local function week_row()
  local row = ""
  for i = 0, 6 do
    local bit = (week_bits >> i) & 1
    local label = DAY_LABELS[((i + view_offset) % 7) + 1]
    row = row .. label .. (bit == 1 and "*" or "-") .. " "
  end
  return row
end

local function draw()
  cp.display.clear()
  cp.display.center(50, "Reading Streak")
  cp.display.center(100, tostring(streak) .. " day streak")
  cp.display.center(140, today_marked() and "Today: Read" or "Today: Not yet")
  cp.display.center(190, "Last 7 days")
  cp.display.center(210, week_row())
  cp.display.center(260, "Confirm = mark today")
  cp.display.center(278, "Up/Down shift view")
  cp.display.center(296, "Back = exit")
  cp.display.refresh()
end

load_state()
rollover_if_needed()

while true do
  draw()
  local btn = cp.input.wait()
  if btn == "back" then
    cp.sys.exit()
  elseif btn == "confirm" then
    mark_today()
  elseif btn == "up" then
    view_offset = (view_offset + 6) % 7
  elseif btn == "down" then
    view_offset = (view_offset + 1) % 7
  end
end
