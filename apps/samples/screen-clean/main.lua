-- Screen Clean: alternating fill pattern to reduce e-ink ghosting.
-- Y coordinates are relative to the content area below the activity header.

cp.display.clear()

local row_width = 45
local solid = string.rep("\226\150\136", row_width)
local blank = string.rep(" ", row_width)
local content_h = cp.display.content_height()

cp.display.center(0, "Screen Clean")

local y = 20
while y <= content_h - 50 do
  cp.display.text(4, y, solid)
  y = y + 14
  if y > content_h - 50 then
    break
  end
  cp.display.text(4, y, blank)
  y = y + 14
end

cp.display.center(content_h - 30, "Run 2-3 times for best results")

cp.sys.log("screen-clean: pattern rendered")
