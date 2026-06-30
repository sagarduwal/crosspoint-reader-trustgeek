-- Countdown: preset break timers (3 / 5 / 10 minutes).

local presets = {
  { label = "3 min", seconds = 3 * 60 },
  { label = "5 min", seconds = 5 * 60 },
  { label = "10 min", seconds = 10 * 60 },
}

local selected = 2

local function format_time(total_sec)
  local mins = math.floor(total_sec / 60)
  local secs = total_sec % 60
  return string.format("%02d:%02d", mins, secs)
end

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

local function draw_menu()
  cp.display.clear()
  cp.display.center(60, "Countdown")
  cp.display.center(90, "Up/Down to choose")
  local y = 140
  for i, preset in ipairs(presets) do
    local prefix = (i == selected) and "> " or "  "
    cp.display.center(y, prefix .. preset.label)
    y = y + 20
  end
  cp.display.center(y + 20, "Confirm to start")
  cp.display.refresh()
end

local function run_countdown(seconds)
  local remaining = seconds
  while remaining > 0 do
    poll_exit()
    cp.display.clear()
    cp.display.center(80, "Countdown")
    cp.display.center(200, format_time(remaining))
    cp.display.center(260, "Back = exit")
    cp.display.refresh()
    wait_ms(1000)
    remaining = remaining - 1
  end
  cp.display.clear()
  cp.display.center(200, "Done!")
  cp.display.refresh()
  wait_ms(1500)
end

while true do
  draw_menu()
  local btn = cp.input.wait()
  if btn == "up" then
    selected = ((selected - 2) % #presets) + 1
  elseif btn == "down" then
    selected = (selected % #presets) + 1
  elseif btn == "confirm" then
    run_countdown(presets[selected].seconds)
  elseif btn == "back" then
    cp.sys.exit()
  end
end
