-- Add mock game items for testing (run once to avoid duplicates).

\echo '--- Inserting Mock Game Items ---'

-- Sentence Match (Advanced)
INSERT INTO game_items (type, level, question, created_by) VALUES
('sentence_match', 'advanced',
 '[
    {
      "sentence_parts": ["Despite", "the", "delay", "the", "team", "completed", "the", "project", "on", "time"],
      "correct_sentence": "Despite the delay the team completed the project on time"
    },
    {
      "sentence_parts": ["No", "sooner", "had", "she", "arrived", "than", "the", "meeting", "began"],
      "correct_sentence": "No sooner had she arrived than the meeting began"
    }
 ]'::jsonb,
 (SELECT user_id FROM users WHERE role IN ('teacher', 'admin') LIMIT 1));

-- Word Match (Intermediate)
INSERT INTO game_items (type, level, question, created_by) VALUES
('word_match', 'intermediate',
 '[
    {"word_pair": ["Abundant", "Plentiful"]},
    {"word_pair": ["Candid", "Honest"]},
    {"word_pair": ["Diligent", "Hardworking"]},
    {"word_pair": ["Evident", "Obvious"]}
 ]'::jsonb,
 (SELECT user_id FROM users WHERE role IN ('teacher', 'admin') LIMIT 1));

-- Image Match (Advanced)
INSERT INTO game_items (type, level, question, created_by) VALUES
('image_match', 'advanced',
 '[
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
 ]'::jsonb,
 (SELECT user_id FROM users WHERE role IN ('teacher', 'admin') LIMIT 1));

\echo '--- Mock game items inserted ---'
