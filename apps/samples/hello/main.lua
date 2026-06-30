-- Hello sample: draws ASCII art and device info via cp.display (Host API v1 subset).

cp.display.clear()

local art = {
  "       .--.       ",
  "      |o_o |      ",
  "      |:_/ |      ",
  "     //   \\ \\     ",
  "    (|     | )    ",
  "   /'\\_   _/`\\   ",
  "   \\___)=(___/   ",
}

local y = 58
for _, line in ipairs(art) do
  cp.display.center(y, line)
  y = y + 14
end

y = y + 10
cp.display.center(y, "~~ Hello, Reader! ~~")
y = y + 22
cp.display.center(y, "CrossPoint App Store")
y = y + 16
cp.display.center(y, "Lua sandbox - Phase 1")

y = y + 28
cp.display.center(y, "Device: " .. cp.sys.device())
y = y + 14
cp.display.center(y, "Host API: " .. cp.sys.version())
y = y + 14
cp.display.center(y, "Uptime: " .. tostring(cp.sys.millis()) .. " ms")

y = y + 24
cp.display.center(y, "*  *  *  *  *")

cp.sys.log("Hello app rendered " .. tostring(#art + 8) .. " display lines")
