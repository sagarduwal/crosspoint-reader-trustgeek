-- Breathing Break: guided 4-7-8 breathing cycles.

local presets = {
  { label = "3 cycles", count = 3 },
  { label = "5 cycles", count = 5 },
  { label = "8 cycles", count = 8 },
}

local phases = {
  { label = "Breathe in", seconds = 4 },
  { label = "Hold", seconds = 7 },
  { label = "Breathe out", seconds = 8 },
}

local selected = 2

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
  cp.display.center(60, "Breathing Break")
  cp.display.center(90, "4-7-8 pattern")
  local y = 140
  for i, preset in ipairs(presets) do
    local prefix = (i == selected) and "> " or "  "
    cp.display.center(y, prefix .. preset.label)
    y = y + 22
  end
  cp.display.center(y + 16, "Confirm to start")
  cp.display.refresh()
end

local function run_phase(label, seconds)
  local remaining = seconds
  while remaining > 0 do
    poll_exit()
    cp.display.clear()
    cp.display.center(120, label)
    cp.display.center(200, tostring(remaining))
    cp.display.center(260, "Back = exit")
    cp.display.refresh()
    wait_ms(1000)
    remaining = remaining - 1
  end
end

local function run_session(cycle_count)
  for cycle = 1, cycle_count do
    cp.display.clear()
    cp.display.center(200, "Cycle " .. tostring(cycle) .. " / " .. tostring(cycle_count))
    cp.display.refresh()
    wait_ms(800)
    for _, phase in ipairs(phases) do
      run_phase(phase.label, phase.seconds)
    end
  end
  cp.display.clear()
  cp.display.center(190, "Nice work.")
  cp.display.center(220, "Ready to read?")
  cp.display.refresh()
  wait_ms(2000)
end

while true do
  draw_menu()
  local btn = cp.input.wait()
  if btn == "up" then
    selected = ((selected - 2) % #presets) + 1
  elseif btn == "down" then
    selected = (selected % #presets) + 1
  elseif btn == "confirm" then
    run_session(presets[selected].count)
  elseif btn == "back" then
    cp.sys.exit()
  end
end
