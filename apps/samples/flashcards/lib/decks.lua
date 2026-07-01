-- Bundled flashcard decks and expansion pools.

local M = {}

M.catalog = {
  {
    id = "literary-terms",
    name = "Literary Terms",
    cards = {
      { front = "Metaphor", back = "Implied comparison" },
      { front = "Simile", back = "Comparison using like/as" },
      { front = "Alliteration", back = "Repeated starting sounds" },
      { front = "Personification", back = "Human traits to objects" },
      { front = "Hyperbole", back = "Deliberate exaggeration" },
      { front = "Irony", back = "Opposite of expectation" },
      { front = "Foreshadowing", back = "Hints at future events" },
      { front = "Symbolism", back = "Object stands for idea" },
    },
    expansion = {
      { front = "Oxymoron", back = "Contradictory terms together" },
      { front = "Onomatopoeia", back = "Word mimics a sound" },
      { front = "Allegory", back = "Story with hidden meaning" },
    },
  },
  {
    id = "vocabulary",
    name = "Vocabulary",
    cards = {
      { front = "Ephemeral", back = "Lasting a very short time" },
      { front = "Ubiquitous", back = "Found everywhere" },
      { front = "Laconic", back = "Using few words" },
      { front = "Perspicacious", back = "Keenly perceptive" },
      { front = "Sanguine", back = "Optimistic and confident" },
      { front = "Obfuscate", back = "Make unclear" },
      { front = "Quixotic", back = "Unrealistically idealistic" },
      { front = "Prosaic", back = "Ordinary or dull" },
    },
    expansion = {
      { front = "Mellifluous", back = "Sweet sounding" },
      { front = "Insouciant", back = "Casually unconcerned" },
      { front = "Recalcitrant", back = "Stubbornly resistant" },
    },
  },
  {
    id = "quotes-authors",
    name = "Quote Authors",
    cards = {
      { front = "To be or not to be", back = "Shakespeare" },
      { front = "It was the best of times", back = "Dickens" },
      { front = "Call me Ishmael", back = "Melville" },
      { front = "All happy families", back = "Tolstoy" },
      { front = "In a hole in the ground", back = "Tolkien" },
      { front = "It is a truth universally", back = "Austen" },
    },
    expansion = {
      { front = "So it goes", back = "Vonnegut" },
      { front = "Stay gold, Ponyboy", back = "Hinton" },
      { front = "The only way out", back = "Plath" },
    },
  },
}

function M.find(id)
  for _, deck in ipairs(M.catalog) do
    if deck.id == id then
      return deck
    end
  end
  return nil
end

return M
