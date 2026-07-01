-- SM-2 spaced repetition scheduler.

local M = {}

function M.schedule(card, quality)
  local ef = card.ef or 2.5
  local interval = card.interval or 0
  local reps = card.reps or 0

  if quality < 3 then
    reps = 0
    interval = 1
  else
    if reps == 0 then
      interval = 1
    elseif reps == 1 then
      interval = 6
    else
      interval = math.max(1, math.floor(interval * ef + 0.5))
    end
    reps = reps + 1
    ef = ef + (0.1 - (5 - quality) * (0.08 + (5 - quality) * 0.02))
    if ef < 1.3 then
      ef = 1.3
    end
  end

  card.ef = ef
  card.interval = interval
  card.reps = reps
  local now = cp.sys.unix_time() or 0
  card.next_review = now + interval * 86400
  return card
end

function M.is_due(card, now)
  now = now or cp.sys.unix_time() or 0
  return (card.next_review or 0) <= now
end

return M
