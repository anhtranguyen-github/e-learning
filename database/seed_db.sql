-- ===================================================
-- SEED DATABASE WITH LARGE TEST DATA
-- ===================================================

\echo '--- Truncating all tables ---'
TRUNCATE TABLE results, exams, exercises, lessons, game_items, users RESTART IDENTITY CASCADE;

\echo '--- Inserting 100 Users ---'
INSERT INTO users (username, password_hash, full_name, role, level)
SELECT 
    'user_' || i,
    md5('password' || i),
    'User ' || i,
    (ARRAY['admin','teacher','student'])[1 + (random()*2)::int],
    (ARRAY['beginner','intermediate','advanced'])[1 + (random()*2)::int]
FROM generate_series(1,100) s(i);

\echo '--- Inserting 100 Lessons ---'
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
    (random()*99 + 1)::int
FROM generate_series(1,100) s(i);

\echo '--- Inserting 100 Exercises ---'
INSERT INTO exercises (lesson_id, title, type, level, question, answer, media_url, created_by)
SELECT 
    (random()*99 + 1)::int,
    'Exercise ' || i,
    (ARRAY['rewrite_sentence','essay','speaking'])[1 + (random()*2)::int],
    (ARRAY['beginner','intermediate','advanced'])[1 + (random()*2)::int],
    'Question text for exercise ' || i,
    'Answer text for exercise ' || i,
    'https://example.com/media' || i,
    (random()*99 + 1)::int
FROM generate_series(1,100) s(i);

\echo '--- Inserting 100 Exams ---'
INSERT INTO exams (lesson_id, title, type, level, question, created_by)
SELECT 
    (random()*99 + 1)::int,
    'Exam ' || i,
    (ARRAY['multiple_choice','fill_blank','order_sentence'])[1 + (random()*2)::int],
    (ARRAY['beginner','intermediate','advanced'])[1 + (random()*2)::int],
    jsonb_build_object('q', 'Question_' || i, 'a', 'Answer_' || i),
    (random()*99 + 1)::int
FROM generate_series(1,100) s(i);

\echo '--- Inserting 100 Results ---'
INSERT INTO results (user_id, target_type, target_id, score, feedback, graded_by)
SELECT
    (random()*99 + 1)::int,
    (ARRAY['exam','exercise'])[1 + (random()*1)::int],
    (random()*99 + 1)::int,
    round((random()*100)::numeric,2),
    'Auto feedback for record ' || i,
    (ARRAY['machine','teacher'])[1 + (random()*1)::int]
FROM generate_series(1,100) s(i);

\echo '--- Inserting 100 Game Items ---'
INSERT INTO game_items (type, question, level, created_by)
SELECT 
    (ARRAY['word_match','sentence_match','image_match'])[1 + (random()*2)::int],
    jsonb_build_object('question', 'Game question ' || i, 'option', 'Option_' || i),
    (ARRAY['beginner','intermediate','advanced'])[1 + (random()*2)::int],
    (random()*99 + 1)::int
FROM generate_series(1,100) s(i);

\echo '--- Data Summary ---'
\echo 'Users:' 
SELECT COUNT(*) AS total FROM users;

\echo 'Lessons:' 
SELECT COUNT(*) AS total FROM lessons;

\echo 'Exercises:' 
SELECT COUNT(*) AS total FROM exercises;

\echo 'Exams:' 
SELECT COUNT(*) AS total FROM exams;

\echo 'Results:' 
SELECT COUNT(*) AS total FROM results;

\echo 'Game Items:' 
SELECT COUNT(*) AS total FROM game_items;

\echo '--- Preview of Each Table ---'
\echo 'Users:'
SELECT user_id, username, role, level FROM users LIMIT 5;

\echo 'Lessons:'
SELECT lesson_id, title, topic, level FROM lessons LIMIT 5;

\echo 'Exercises:'
SELECT exercise_id, title, type, level FROM exercises LIMIT 5;

\echo 'Exams:'
SELECT exam_id, title, type, level FROM exams LIMIT 5;

\echo 'Results:'
SELECT result_id, user_id, target_type, score FROM results LIMIT 5;

\echo 'Game Items:'
SELECT game_id, type, level FROM game_items LIMIT 5;

\echo '--- Seeding Complete ---'
