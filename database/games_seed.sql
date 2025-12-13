
\echo '--- Inserting Game Items ---'

-- Sentence Match Games
INSERT INTO game_items (type, level, question, created_by) VALUES
('sentence_match', 'beginner',
 '[
    {
      "sentence_parts": ["The", "cat", "is", "sleeping", "on", "the", "mat"],
      "correct_sentence": "The cat is sleeping on the mat"
    },
    {
       "sentence_parts": ["I", "like", "to", "play", "football"],
       "correct_sentence": "I like to play football"
    }
 ]'::jsonb, 1),

('sentence_match', 'intermediate',
 '[
    {
      "sentence_parts": ["Although", "it", "rained", "we", "went", "out"],
      "correct_sentence": "Although it rained we went out"
    },
    {
       "sentence_parts": ["If", "I", "were", "you", "I", "would", "study"],
       "correct_sentence": "If I were you I would study"
    }
 ]'::jsonb, 1);

-- Word Match Games
INSERT INTO game_items (type, level, question, created_by) VALUES
('word_match', 'beginner',
 '[
    {"word_pair": ["Apple", "Fruit"]},
    {"word_pair": ["Dog", "Animal"]},
    {"word_pair": ["Car", "Vehicle"]},
    {"word_pair": ["Red", "Color"]}
 ]'::jsonb, 1),

('word_match', 'intermediate',
 '[
    {"word_pair": ["Benevolent", "Kind"]},
    {"word_pair": ["Cacophony", "Noise"]},
    {"word_pair": ["Ephemeral", "Short-lived"]},
    {"word_pair": ["Lethargic", "Sluggish"]}
 ]'::jsonb, 1);

-- Picture Match Games (Using local paths)
INSERT INTO game_items (type, level, question, created_by) VALUES
('image_match', 'beginner',
 '[
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
 ]'::jsonb, 1);
