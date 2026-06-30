-- Break Tips: static 20/5 reading break guide.
-- Y coordinates are relative to the content area below the activity header.

cp.display.clear()

local tips = {
  "Reading Breaks",
  "",
  "20 min read / 5 min rest",
  "",
  "- Stand and stretch your legs",
  "- Look away to rest your eyes",
  "- Drink some water",
  "- Roll shoulders and neck gently",
  "- Take a few deep breaths",
  "",
  "Install Pomodoro for live timer",
}

local y = 0
for _, line in ipairs(tips) do
  cp.display.center(y, line)
  y = y + 16
end

cp.sys.log("break-tips: rendered " .. tostring(#tips) .. " lines")
