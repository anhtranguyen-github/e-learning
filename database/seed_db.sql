-- ===================================================
-- SEED DATABASE WITH LARGE TEST DATA
-- ===================================================

\echo '--- Truncating all tables ---'
TRUNCATE TABLE results, exams, exercises, lessons, game_items, users RESTART IDENTITY CASCADE;

\echo '--- Inserting Users ---'
-- Admin and Student
INSERT INTO users (username, password_hash, full_name, role, level) VALUES
('admin', 'admin', 'Administrator', 'admin', 'advanced'),
('student', 'password', 'Student User', 'student', 'beginner');

-- Additional random users
INSERT INTO users (username, password_hash, full_name, role, level)
SELECT 
    'user_' || i,
    'password', -- Simple password for testing
    'User ' || i,
    (ARRAY['student'])[1],
    (ARRAY['beginner','intermediate','advanced'])[1 + (random()*2)::int]
FROM generate_series(1,10) s(i);

\echo '--- Inserting Lessons ---'
INSERT INTO lessons (title, topic, level, video_url, audio_url, text_content, vocabulary, grammar, created_by)
SELECT 
    'Lesson ' || i,
    'Topic ' || ((i % 10) + 1),
    (ARRAY['beginner','intermediate','advanced'])[1 + (random()*2)::int],
    'https://example.com/video' || i,
    'https://example.com/audio' || i,
    'This is the lesson content number ' || i,
    jsonb_build_object('word', 'example_' || i, 'meaning', 'meaning_' || i),
    jsonb_build_object('rule', 'grammar_rule_' || i, 'example', 'example_sentence_' || i),
    1 -- Created by admin (id 1)
FROM generate_series(1,20) s(i);

\echo '--- Inserting Exercises ---'
-- 1. Legacy Single Question Exercises
INSERT INTO exercises (lesson_id, title, type, level, question, answer, media_url, created_by)
SELECT 
    (random()*19 + 1)::int,
    'Legacy Exercise ' || i,
    (ARRAY['rewrite_sentence','essay','speaking'])[1 + (random()*2)::int],
    (ARRAY['beginner','intermediate','advanced'])[1 + (random()*2)::int],
    'Legacy Question text for exercise ' || i,
    'Answer text for exercise ' || i,
    'https://example.com/media' || i,
    1
FROM generate_series(1,10) s(i);

-- 2. Multi-Question Exercises (New Format)
INSERT INTO exercises (lesson_id, title, type, level, questions, created_by)
VALUES 
(1, 'Multi-Q Exercise 1', 'multiple_choice', 'beginner', 
 '[
    {"text": "What is 2+2?", "type": "multiple_choice", "options": ["3", "4", "5"], "answer": "4", "explanation": "Math"},
    {"text": "What is 5+5?", "type": "multiple_choice", "options": ["10", "20"], "answer": "10", "explanation": "Math"}
 ]'::jsonb, 1),
(1, 'Multi-Q Exercise 2', 'fill_in_blank', 'intermediate',
 '[
    {"text": "The sky is ___.", "type": "fill_in_blank", "options": [], "answer": "blue", "explanation": "Nature"},
    {"text": "Grass is ___.", "type": "fill_in_blank", "options": [], "answer": "green", "explanation": "Nature"}
 ]'::jsonb, 1);

\echo '--- Inserting Exams ---'
INSERT INTO exams (lesson_id, title, type, level, question, created_by)
SELECT 
    (random()*19 + 1)::int,
    'Exam ' || i,
    (ARRAY['multiple_choice','fill_blank','order_sentence'])[1 + (random()*2)::int],
    (ARRAY['beginner','intermediate','advanced'])[1 + (random()*2)::int],
    jsonb_build_array(
        jsonb_build_object(
            'text', 'Exam Q1 for exam ' || i,
            'type', 'multiple_choice',
            'options', jsonb_build_array('A', 'B', 'C'),
            'answer', 'A',
            'explanation', 'Exp 1'
        ),
        jsonb_build_object(
            'text', 'Exam Q2 for exam ' || i,
            'type', 'multiple_choice',
            'options', jsonb_build_array('X', 'Y', 'Z'),
            'answer', 'Z',
            'explanation', 'Exp 2'
        )
    ),
    1
FROM generate_series(1,10) s(i);

\echo '--- Data Summary ---'
\echo 'Users:' 
SELECT COUNT(*) AS total FROM users;

\echo 'Lessons:' 
SELECT COUNT(*) AS total FROM lessons;

\echo 'Exercises:' 
SELECT COUNT(*) AS total FROM exercises;

\echo 'Exams:' 
SELECT COUNT(*) AS total FROM exams;

\echo '--- Seeding Complete ---'
