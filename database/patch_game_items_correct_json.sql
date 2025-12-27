-- Update existing game_items with correct JSON payloads by type and level.

\echo '--- Updating game_items JSON payloads ---'

UPDATE game_items
SET question = '[
  {
    "sentence_parts": ["The", "cat", "is", "sleeping", "on", "the", "mat"],
    "correct_sentence": "The cat is sleeping on the mat"
  },
  {
    "sentence_parts": ["I", "like", "to", "play", "football"],
    "correct_sentence": "I like to play football"
  }
]'::jsonb
WHERE type = 'sentence_match' AND level = 'beginner';

UPDATE game_items
SET question = '[
  {
    "sentence_parts": ["Although", "it", "rained", "we", "went", "out"],
    "correct_sentence": "Although it rained we went out"
  },
  {
    "sentence_parts": ["If", "I", "were", "you", "I", "would", "study"],
    "correct_sentence": "If I were you I would study"
  }
]'::jsonb
WHERE type = 'sentence_match' AND level = 'intermediate';

UPDATE game_items
SET question = '[
  {
    "sentence_parts": ["Had", "I", "known", "about", "the", "traffic", "I", "would", "have", "left", "earlier"],
    "correct_sentence": "Had I known about the traffic I would have left earlier"
  },
  {
    "sentence_parts": ["Not", "only", "did", "he", "win", "but", "he", "broke", "the", "record"],
    "correct_sentence": "Not only did he win but he broke the record"
  }
]'::jsonb
WHERE type = 'sentence_match' AND level = 'advanced';

UPDATE game_items
SET question = '[
  {"word_pair": ["Apple", "Fruit"]},
  {"word_pair": ["Dog", "Animal"]},
  {"word_pair": ["Car", "Vehicle"]},
  {"word_pair": ["Red", "Color"]}
]'::jsonb
WHERE type = 'word_match' AND level = 'beginner';

UPDATE game_items
SET question = '[
  {"word_pair": ["Benevolent", "Kind"]},
  {"word_pair": ["Cacophony", "Noise"]},
  {"word_pair": ["Ephemeral", "Short-lived"]},
  {"word_pair": ["Lethargic", "Sluggish"]}
]'::jsonb
WHERE type = 'word_match' AND level = 'intermediate';

UPDATE game_items
SET question = '[
  {"word_pair": ["Ubiquitous", "Everywhere"]},
  {"word_pair": ["Mellifluous", "Sweet-sounding"]},
  {"word_pair": ["Obfuscate", "Confuse"]},
  {"word_pair": ["Pernicious", "Harmful"]}
]'::jsonb
WHERE type = 'word_match' AND level = 'advanced';

UPDATE game_items
SET question = '[
  {
    "image_url": "apple.png",
    "word": "Apple",
    "options": ["Apple", "Banana", "Orange", "Grape"]
  },
  {
    "image_url": "car.png",
    "word": "Car",
    "options": ["Car", "Bus", "Bike", "Train"]
  },
  {
    "image_url": "dog.png",
    "word": "Dog",
    "options": ["Dog", "Cat", "Bird", "Fish"]
  }
]'::jsonb
WHERE type = 'image_match' AND level = 'beginner';

UPDATE game_items
SET question = '[
  {
    "image_url": "elephant.png",
    "word": "Elephant",
    "options": ["Elephant", "Rhino", "Hippo", "Giraffe"]
  },
  {
    "image_url": "penguin.png",
    "word": "Penguin",
    "options": ["Penguin", "Ostrich", "Eagle", "Owl"]
  },
  {
    "image_url": "dolphin.png",
    "word": "Dolphin",
    "options": ["Dolphin", "Shark", "Whale", "Seal"]
  }
]'::jsonb
WHERE type = 'image_match' AND level = 'intermediate';

UPDATE game_items
SET question = '[
  {
    "image_url": "volcano.png",
    "word": "Volcano",
    "options": ["Volcano", "Canyon", "Island", "Forest"]
  },
  {
    "image_url": "satellite.png",
    "word": "Satellite",
    "options": ["Satellite", "Rocket", "Astronaut", "Telescope"]
  },
  {
    "image_url": "canyon.png",
    "word": "Canyon",
    "options": ["Canyon", "Valley", "Mountain", "River"]
  }
]'::jsonb
WHERE type = 'image_match' AND level = 'advanced';

\echo '--- Game items JSON updated ---'
