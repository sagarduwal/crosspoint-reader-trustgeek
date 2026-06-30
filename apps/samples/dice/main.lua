-- Dice Roller: press Confirm to roll a six-sided die.

local ascii = {
  [1] = {
    "   ",
    " * ",
    "   ",
  },
  [2] = {
    "*  ",
    "   ",
    "  *",
  },
  [3] = {
    "*  ",
    " * ",
    "  *",
  },
  [4] = {
    "* *",
    "   ",
    "* *",
  },
  [5] = {
    "* *",
    " * ",
    "* *",
  },
  [6] = {
    "* *",
    "* *",
    "* *",
  },
}

local function draw_face(value)
  cp.display.clear()
  cp.display.center(60, "Dice Roller")
  local y = 150
  for _, line in ipairs(ascii[value]) do
    cp.display.center(y, line)
    y = y + 24
  end
  cp.display.center(y + 40, "You rolled: " .. tostring(value))
  cp.display.center(y + 70, "Confirm = roll again")
  cp.display.center(y + 90, "Back = exit")
  cp.display.refresh()
end

math.randomseed(cp.sys.millis())

while true do
  local roll = math.random(1, 6)
  draw_face(roll)
  cp.sys.log("dice: rolled " .. tostring(roll))
  local btn = cp.input.wait()
  if btn == "back" then
    cp.sys.exit()
  end
end
