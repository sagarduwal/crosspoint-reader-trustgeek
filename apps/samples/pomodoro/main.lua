-- Pomodoro: 25 min focus / 5 min break cycles.

local FOCUS_SEC = 25 * 60
local BREAK_SEC = 5 * 60

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

local function run_phase(label, total_sec)
  local remaining = total_sec
  local paused = false
  while remaining > 0 do
    local btn = poll_exit()
    if btn == "confirm" then
      paused = not paused
      wait_ms(200)
    end
    if not paused then
      cp.display.clear()
      cp.display.center(80, "Pomodoro")
      cp.display.center(120, label)
      cp.display.center(200, format_time(remaining))
      cp.display.center(260, paused and "PAUSED" or "Confirm = pause")
      cp.display.center(290, "Back = exit")
      local done = cp.settings.get("completed_count", 0)
      cp.display.center(320, "Completed: " .. tostring(done))
      cp.display.refresh()
      wait_ms(1000)
      remaining = remaining - 1
    else
      cp.display.clear()
      cp.display.center(200, "PAUSED")
      cp.display.center(230, format_time(remaining))
      cp.display.refresh()
      wait_ms(100)
    end
  end
end

local completed = cp.settings.get("completed_count", 0)

while true do
  run_phase("FOCUS", FOCUS_SEC)
  completed = completed + 1
  cp.settings.set("completed_count", completed)
  run_phase("BREAK", BREAK_SEC)
end
