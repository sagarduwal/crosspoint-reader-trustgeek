-- World Clock: local time plus UTC, EST, CST, and NPT.

local zones = {
  { label = "Local", offset_q = nil },
  { label = "UTC", offset_q = 48 },
  { label = "EST", offset_q = 28 },
  { label = "CST", offset_q = 24 },
  { label = "NPT", offset_q = 71 },
}

local selected = 1
local use_12h = false

local function poll_exit()
  local btn = cp.input.poll()
  if btn == "back" then
    cp.sys.exit()
  end
  return btn
end

local function wait_ms(ms)
  local elapsed = 0
  while elapsed < ms do
    poll_exit()
    cp.sys.sleep_ms(50)
    elapsed = elapsed + 50
  end
end

local function zone_time(zone)
  if zone.offset_q == nil then
    return cp.sys.format_time(use_12h)
  end
  return cp.sys.format_time_offset(zone.offset_q, use_12h)
end

local function draw()
  cp.display.clear()
  cp.display.center(40, "World Clock")
  if not cp.sys.time_synced() then
    cp.display.center(80, "Clock not synced.")
    cp.display.center(100, "Settings > Status Bar")
    cp.display.center(118, "Sync clock now")
    cp.display.center(160, zones[selected].label)
    cp.display.center(200, "--:--")
    cp.display.center(260, "Back = exit")
    cp.display.refresh()
    return
  end

  local y = 80
  for i, zone in ipairs(zones) do
    local prefix = (i == selected) and "> " or "  "
    local time_str = zone_time(zone) or "--:--"
    local line = prefix .. zone.label
    while #line < 12 do
      line = line .. " "
    end
    cp.display.center(y, line .. time_str)
    y = y + 22
  end
  cp.display.center(y + 10, "Confirm = 12h/24h")
  cp.display.center(y + 28, "Back = exit")
  cp.display.refresh()
end

if cp.settings.global_get("clock_format") == 1 then
  use_12h = true
end

while true do
  draw()
  local btn = cp.input.poll()
  if btn == "back" then
    cp.sys.exit()
  elseif btn == "up" then
    selected = ((selected - 2) % #zones) + 1
    wait_ms(150)
  elseif btn == "down" then
    selected = (selected % #zones) + 1
    wait_ms(150)
  elseif btn == "confirm" then
    use_12h = not use_12h
    wait_ms(150)
  else
    wait_ms(1000)
  end
end
