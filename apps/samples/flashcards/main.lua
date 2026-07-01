-- Flashcards: SM-2 spaced repetition with multiple decks.

local sm2 = require("sm2")
local deck_defs = require("decks")

local GRADES = { "Again", "Hard", "Good", "Easy" }
local GRADE_QUALITY = { 0, 2, 3, 5 }

local screen = "menu"
local deck_ids = {}
local deck_cards = {}
local selected = 1
local review_queue = {}
local review_index = 1
local flipped = false
local grade_selected = 3
local manager_mode = false
local pool_selected = {}
local pool_cursor = 1

local function deck_path(id)
  return "deck_" .. id .. ".txt"
end

local function serialize_card(card)
  return table.concat({
    card.front,
    card.back,
    tostring(card.ef or 2.5),
    tostring(card.interval or 0),
    tostring(card.reps or 0),
    tostring(card.next_review or 0),
  }, "\t")
end

local function deserialize_card(line)
  local front, back, ef, interval, reps, next_review = line:match("([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)")
  if not front then
    return nil
  end
  return {
    front = front,
    back = back,
    ef = tonumber(ef) or 2.5,
    interval = tonumber(interval) or 0,
    reps = tonumber(reps) or 0,
    next_review = tonumber(next_review) or 0,
  }
end

local function load_deck(id)
  local cards = {}
  if cp.fs.exists(deck_path(id)) then
    local data = cp.fs.read(deck_path(id))
    if data then
      for line in data:gmatch("[^\r\n]+") do
        local card = deserialize_card(line)
        if card then
          cards[#cards + 1] = card
        end
      end
    end
  end
  if #cards == 0 then
    local def = deck_defs.find(id)
    if def then
      for _, card in ipairs(def.cards) do
        cards[#cards + 1] = {
          front = card.front,
          back = card.back,
          ef = 2.5,
          interval = 0,
          reps = 0,
          next_review = 0,
        }
      end
      cp.fs.write(deck_path(id), table.concat((function()
        local lines = {}
        for i, card in ipairs(cards) do
          lines[i] = serialize_card(card)
        end
        return lines
      end)(), "\n"))
    end
  end
  deck_cards[id] = cards
end

local function save_deck(id)
  local cards = deck_cards[id] or {}
  local lines = {}
  for i, card in ipairs(cards) do
    lines[i] = serialize_card(card)
  end
  cp.fs.write(deck_path(id), table.concat(lines, "\n"))
end

local function load_deck_ids()
  deck_ids = {}
  if cp.fs.exists("decks.txt") then
    local data = cp.fs.read("decks.txt")
    if data then
      for line in data:gmatch("[^\r\n]+") do
        deck_ids[#deck_ids + 1] = line
      end
    end
  end
  if #deck_ids == 0 then
    for _, def in ipairs(deck_defs.catalog) do
      deck_ids[#deck_ids + 1] = def.id
    end
    cp.fs.write("decks.txt", table.concat(deck_ids, "\n"))
  end
  for _, id in ipairs(deck_ids) do
    load_deck(id)
  end
end

local function deck_name(id)
  local def = deck_defs.find(id)
  return def and def.name or id
end

local function due_count(id)
  local count = 0
  for _, card in ipairs(deck_cards[id] or {}) do
    if sm2.is_due(card) then
      count = count + 1
    end
  end
  return count
end

local function build_review_queue(id)
  review_queue = {}
  for _, card in ipairs(deck_cards[id] or {}) do
    if sm2.is_due(card) then
      review_queue[#review_queue + 1] = card
    end
  end
  review_index = 1
  flipped = false
  grade_selected = 3
end

local function truncate(text, max_len)
  if #text <= max_len then
    return text
  end
  return text:sub(1, max_len - 3) .. "..."
end

local function draw_menu()
  cp.display.clear()
  cp.display.center(40, "Flashcards")
  cp.display.center(60, manager_mode and "Deck manager" or "Select deck")
  local y = 90
  for i, id in ipairs(deck_ids) do
    local prefix = (i == selected) and "> " or "  "
    local due = due_count(id)
    cp.display.center(y, prefix .. deck_name(id) .. " (" .. tostring(due) .. " due)")
    y = y + 20
  end
  cp.display.center(y + 10, manager_mode and "Confirm=review Right=add" or "Confirm=review Left=manage")
  cp.display.center(y + 28, "Back=exit")
  cp.display.refresh()
end

local function draw_review()
  local card = review_queue[review_index]
  cp.display.clear()
  cp.display.center(40, deck_name(deck_ids[selected]))
  cp.display.center(80, flipped and "Back side" or "Front")
  cp.display.center(140, truncate(flipped and card.back or card.front, 42))
  cp.display.center(220, flipped and "Pick grade:" or "Confirm=flip")
  if flipped then
    local y = 250
    for i, label in ipairs(GRADES) do
      local prefix = (i == grade_selected) and "> " or "  "
      cp.display.center(y, prefix .. label)
      y = y + 18
    end
  end
  cp.display.center(330, "Card " .. tostring(review_index) .. "/" .. tostring(#review_queue))
  cp.display.refresh()
end

local function draw_done()
  cp.display.clear()
  cp.display.center(200, "All caught up!")
  cp.display.refresh()
end

local function draw_manager_delete()
  local id = deck_ids[selected]
  local cards = deck_cards[id] or {}
  cp.display.clear()
  cp.display.center(40, "Remove cards")
  cp.display.center(60, deck_name(id))
  local y = 90
  for i, card in ipairs(cards) do
    if i > 8 then
      break
    end
    local prefix = (i == pool_cursor) and "> " or "  "
    cp.display.center(y, prefix .. truncate(card.front, 36))
    y = y + 18
  end
  cp.display.center(y + 10, "Confirm=delete Back=cancel")
  cp.display.refresh()
end

local function draw_add_pool()
  local id = deck_ids[selected]
  local def = deck_defs.find(id)
  local pool = def and def.expansion or {}
  cp.display.clear()
  cp.display.center(40, "Add cards")
  cp.display.center(60, deck_name(id))
  local y = 90
  for i, card in ipairs(pool) do
    local mark = pool_selected[i] and "[x]" or "[ ]"
    local prefix = (i == pool_cursor) and "> " or "  "
    cp.display.center(y, prefix .. mark .. " " .. truncate(card.front, 30))
    y = y + 18
  end
  cp.display.center(y + 10, "Confirm=toggle Right=save")
  cp.display.refresh()
end

local function start_add_pool()
  pool_selected = {}
  pool_cursor = 1
  screen = "add_pool"
end

local function commit_add_pool()
  local id = deck_ids[selected]
  local def = deck_defs.find(id)
  local pool = def and def.expansion or {}
  local cards = deck_cards[id] or {}
  for i, card in ipairs(pool) do
    if pool_selected[i] then
      cards[#cards + 1] = {
        front = card.front,
        back = card.back,
        ef = 2.5,
        interval = 0,
        reps = 0,
        next_review = 0,
      }
    end
  end
  deck_cards[id] = cards
  save_deck(id)
  screen = "menu"
end

local function delete_selected_card()
  local id = deck_ids[selected]
  local cards = deck_cards[id] or {}
  if cards[pool_cursor] then
    table.remove(cards, pool_cursor)
    if pool_cursor > #cards then
      pool_cursor = math.max(1, #cards)
    end
    deck_cards[id] = cards
    save_deck(id)
  end
  if #cards == 0 then
    screen = "menu"
  end
end

load_deck_ids()

while true do
  if screen == "menu" then
    draw_menu()
    local btn = cp.input.wait()
    if btn == "back" then
      cp.sys.exit()
    elseif btn == "up" and #deck_ids > 0 then
      selected = ((selected - 2) % #deck_ids) + 1
    elseif btn == "down" and #deck_ids > 0 then
      selected = (selected % #deck_ids) + 1
    elseif btn == "left" then
      manager_mode = not manager_mode
    elseif btn == "right" and manager_mode then
      start_add_pool()
    elseif btn == "confirm" and #deck_ids > 0 then
      if manager_mode then
        pool_cursor = 1
        screen = "manager"
      else
        build_review_queue(deck_ids[selected])
        if #review_queue == 0 then
          screen = "done"
        else
          screen = "review"
        end
      end
    end
  elseif screen == "review" then
    draw_review()
    local btn = cp.input.wait()
    if btn == "back" then
      screen = "menu"
    elseif btn == "confirm" and not flipped then
      flipped = true
    elseif flipped then
      if btn == "up" then
        grade_selected = ((grade_selected - 2) % #GRADES) + 1
      elseif btn == "down" then
        grade_selected = (grade_selected % #GRADES) + 1
      elseif btn == "confirm" then
        local card = review_queue[review_index]
        sm2.schedule(card, GRADE_QUALITY[grade_selected])
        save_deck(deck_ids[selected])
        review_index = review_index + 1
        flipped = false
        grade_selected = 3
        if review_index > #review_queue then
          screen = "done"
        end
      end
    end
  elseif screen == "done" then
    draw_done()
    cp.sys.sleep_ms(1500)
    screen = "menu"
  elseif screen == "manager" then
    draw_manager_delete()
    local btn = cp.input.wait()
    if btn == "back" then
      screen = "menu"
    elseif btn == "up" then
      local count = #(deck_cards[deck_ids[selected]] or {})
      if count > 0 then
        pool_cursor = ((pool_cursor - 2) % count) + 1
      end
    elseif btn == "down" then
      local count = #(deck_cards[deck_ids[selected]] or {})
      if count > 0 then
        pool_cursor = (pool_cursor % count) + 1
      end
    elseif btn == "confirm" then
      delete_selected_card()
    end
  elseif screen == "add_pool" then
    draw_add_pool()
    local btn = cp.input.wait()
    if btn == "back" then
      screen = "menu"
    elseif btn == "up" then
      local def = deck_defs.find(deck_ids[selected])
      local count = def and #def.expansion or 0
      if count > 0 then
        pool_cursor = ((pool_cursor - 2) % count) + 1
      end
    elseif btn == "down" then
      local def = deck_defs.find(deck_ids[selected])
      local count = def and #def.expansion or 0
      if count > 0 then
        pool_cursor = (pool_cursor % count) + 1
      end
    elseif btn == "confirm" then
      pool_selected[pool_cursor] = not pool_selected[pool_cursor]
    elseif btn == "right" then
      commit_add_pool()
    end
  end
end
