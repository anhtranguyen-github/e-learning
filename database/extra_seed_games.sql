
\echo '--- Inserting Additional Game Items ---'

-- 1. Word Match (Advanced)
INSERT INTO game_items (type, level, question, created_by) VALUES
('word_match', 'advanced',
 '[
    {"word_pair": ["Ubiquitous", "Everywhere"]},
    {"word_pair": ["Mellifluous", "Sweet-sounding"]},
    {"word_pair": ["Obfuscate", "Confuse"]},
    {"word_pair": ["Pernicious", "Harmful"]},
    {"word_pair": ["Sycohant", "Flatterer"]}
 ]'::jsonb, 
 (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1));

-- 2. Word Match (Beginner) - Colors & Shapes
INSERT INTO game_items (type, level, question, created_by) VALUES
('word_match', 'beginner',
 '[
    {"word_pair": ["Blue", "Sky color"]},
    {"word_pair": ["Yellow", "Sun color"]},
    {"word_pair": ["Circle", "Round"]},
    {"word_pair": ["Square", "Four sides"]},
    {"word_pair": ["Triangle", "Three sides"]}
 ]'::jsonb, 
 (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1));

-- 3. Sentence Match (Beginner) - Simple Routines
INSERT INTO game_items (type, level, question, created_by) VALUES
('sentence_match', 'beginner',
 '[
    {
      "sentence_parts": ["I", "brush", "my", "teeth", "every", "morning"],
      "correct_sentence": "I brush my teeth every morning"
    },
    {
       "sentence_parts": ["She", "drinks", "coffee", "at", "breakfast"],
       "correct_sentence": "She drinks coffee at breakfast"
    },
    {
       "sentence_parts": ["They", "walk", "to", "school", "together"],
       "correct_sentence": "They walk to school together"
    }
 ]'::jsonb, 
 (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1));

-- 4. Sentence Match (Advanced) - Complex Structures
INSERT INTO game_items (type, level, question, created_by) VALUES
('sentence_match', 'advanced',
 '[
    {
      "sentence_parts": ["Had", "I", "known", "about", "the", "traffic", "I", "would", "have", "left", "earlier"],
      "correct_sentence": "Had I known about the traffic I would have left earlier"
    },
    {
       "sentence_parts": ["Not", "only", "did", "he", "win", "but", "he", "broke", "the", "record"],
       "correct_sentence": "Not only did he win but he broke the record"
    }
 ]'::jsonb, 
 (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1));

-- 5. Image Match (Intermediate) - Animals (Simulated)
INSERT INTO game_items (type, level, question, created_by) VALUES
('image_match', 'intermediate',
 '[
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
 ]'::jsonb, 
 (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1));

\echo '--- Inserted 5 additional games ---'
