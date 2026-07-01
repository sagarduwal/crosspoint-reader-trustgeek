-- Baby Flashcards: high-contrast 0-3 month visual cards with manual navigation.

local catalog = require("cards")

local cards = catalog.cards
local index = 1

-- Must match OUTPUT_SIZE in apiserver/scripts/rasterize_card_assets.py
local ASSET_PX = 800
local FOOTER_H = 48

local function wrap_index(next_index)
  local count = #cards
  if count == 0 then
    return 1
  end
  if next_index < 1 then
    return count
  end
  if next_index > count then
    return 1
  end
  return next_index
end

local function draw_card()
  local screen_w = cp.display.width()
  local screen_h = cp.display.height()
  local card = cards[index]
  local area_h = screen_h - FOOTER_H

  local scale = math.min(screen_w / ASSET_PX, area_h / ASSET_PX)
  if scale > 1 then
    scale = 1
  end
  local draw_size = math.floor(ASSET_PX * scale)
  local x = math.floor((screen_w - draw_size) / 2)
  local y = math.floor((area_h - draw_size) / 2)

  cp.display.clear()
  cp.display.bmp(card.file, x, y, draw_size, draw_size)
  cp.display.center(area_h + 10, index .. " / " .. #cards)
  cp.display.center(area_h + 30, "Up/Left prev  Down/Right next")
  cp.display.refresh()
end

if #cards == 0 then
  cp.display.clear()
  cp.display.center(200, "No cards found")
  cp.display.refresh()
  cp.sys.exit()
end

while true do
  draw_card()
  local btn = cp.input.wait()
  if btn == "back" then
    cp.sys.exit()
  elseif btn == "up" or btn == "left" then
    index = wrap_index(index - 1)
  elseif btn == "down" or btn == "right" or btn == "confirm" then
    index = wrap_index(index + 1)
  end
end
