-- Daily Quote: reading-themed quote selected by uptime-day index.
-- Y coordinates are relative to the content area below the activity header.

cp.display.clear()

local quotes = {
  { text = "A reader lives a thousand lives before he dies.", author = "George R.R. Martin" },
  { text = "There is no friend as loyal as a book.", author = "Ernest Hemingway" },
  { text = "Books are a uniquely portable magic.", author = "Stephen King" },
  { text = "Reading is dreaming with open eyes.", author = "Yoyo" },
  { text = "Once you learn to read, you will be forever free.", author = "Frederick Douglass" },
  { text = "The reading of all good books is like conversation with the finest minds.", author = "Descartes" },
  { text = "A room without books is like a body without a soul.", author = "Cicero" },
  { text = "Today a reader, tomorrow a leader.", author = "Margaret Fuller" },
  { text = "I have always imagined that Paradise will be a kind of library.", author = "Jorge Luis Borges" },
  { text = "Reading furnishes the mind only with materials of knowledge.", author = "John Locke" },
  { text = "Wear the old coat and buy the new book.", author = "Austin Phelps" },
  { text = "The more that you read, the more things you will know.", author = "Dr. Seuss" },
}

local function wrap_line(text, max_len)
  local lines = {}
  local remaining = text
  while #remaining > max_len do
    local cut = max_len
    local space = remaining:sub(1, max_len):match(".*() ")
    if space and space > 20 then
      cut = space - 1
    end
    lines[#lines + 1] = remaining:sub(1, cut)
    remaining = remaining:sub(cut + 1):match("^%s*(.*)$") or ""
  end
  if #remaining > 0 then
    lines[#lines + 1] = remaining
  end
  return lines
end

local index = (math.floor(cp.sys.millis() / 86400000) % #quotes) + 1
local quote = quotes[index]

local y = 0
cp.display.center(y, "Daily Quote")
y = y + 28

for _, line in ipairs(wrap_line(quote.text, 42)) do
  cp.display.center(y, line)
  y = y + 14
end

y = y + 10
cp.display.center(y, "- " .. quote.author)

cp.sys.log("daily-quote: index=" .. tostring(index))
