-- ===================================================
-- SEED DATABASE WITH LARGE TEST DATA
-- ===================================================



-- psql -d english_learning -f database/patch_mock_games.sql
-- psql -d english_learning -f database/patch_mock_exercise.sql

\echo '--- Truncating all tables ---'
TRUNCATE TABLE results, exams, exercises, lessons, game_items, users RESTART IDENTITY CASCADE;

\echo '--- Inserting Users ---'
-- Admin and Student
INSERT INTO users (username, password_hash, full_name, role, level) VALUES
('admin', 'admin', 'Administrator', 'admin', 'advanced'),
('teacher', 'teacher123', 'Teacher User', 'teacher', 'advanced'),
('student1', 'password', 'Student One', 'student', 'beginner'),
('student2', 'password', 'Student Two', 'student', 'intermediate'),
('student3', 'password', 'Student Three', 'student', 'advanced');

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
    jsonb_build_array(jsonb_build_object('word', 'example_' || i, 'meaning', 'meaning_' || i)),
    jsonb_build_array(jsonb_build_object('rule', 'grammar_rule_' || i, 'example', 'example_sentence_' || i)),
    1 -- Created by admin (id 1)
FROM generate_series(1,20) s(i);

\echo '--- Inserting Exercises ---'
-- 1. Single Question Exercises
INSERT INTO exercises (lesson_id, title, type, level, questions, media_url, created_by)
SELECT 
    (random()*19 + 1)::int,
    'Exercise ' || i,
    (ARRAY['rewrite_sentence','essay','speaking'])[1 + (random()*2)::int],
    (ARRAY['beginner','intermediate','advanced'])[1 + (random()*2)::int],
    jsonb_build_array(jsonb_build_object(
        'text', 'Question text for exercise ' || i,
        'answer', 'Answer text for exercise ' || i,
        'type', 'text' -- Default or derived
    )),
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

\echo '--- Inserting Subjective Exam ---'
INSERT INTO exams (lesson_id, title, type, level, question, created_by)
VALUES (1, 'Subjective Exam 1', 'mixed', 'intermediate', 
    '[
        {"text": "Explain the importance of grammar.", "type": "essay", "answer": "", "explanation": "Subjective"},
        {"text": "Read this paragraph aloud.", "type": "speaking", "answer": "", "explanation": "Subjective"}
    ]'::jsonb, 1);

\echo '--- Inserting Results ---'
-- 1. Graded Exam Result (Objective)
INSERT INTO results (user_id, target_type, target_id, score, user_answer, feedback, status)
VALUES (2, 'exam', 1, 100.0, 'A^Z', 'You got 2 out of 2 correct.', 'graded');

-- 2. Pending Exam Result (Subjective)
-- Assuming the subjective exam inserted above has ID 11 (since 10 were generated before)
INSERT INTO results (user_id, target_type, target_id, score, user_answer, feedback, status)
VALUES (2, 'exam', 11, 0.0, 'My essay answer^Audio file path', 'Pending instructor review', 'pending');

-- 3. Graded Exercise Result
INSERT INTO results (user_id, target_type, target_id, score, user_answer, feedback, status)
VALUES (2, 'exercise', 1, 100.0, 'Answer text', 'Correct!', 'graded');

-- 4. Pending Exercise Result
INSERT INTO results (user_id, target_type, target_id, score, user_answer, feedback, status)
VALUES (2, 'exercise', 2, 0.0, 'My essay', 'Pending instructor review', 'pending');

\echo 'Results:'
SELECT COUNT(*) AS total FROM results;

\echo '--- Inserting Chat Messages ---'
INSERT INTO chat_messages (sender_id, receiver_id, content, message_type) VALUES
(1, 2, 'Hello Student, how are you?', 'TEXT'),
(2, 1, 'I am doing well, thank you!', 'TEXT'),
(1, 2, 'Do you have any questions about the lesson?', 'TEXT'),
(2, 1, 'Not yet, I will ask if I do.', 'TEXT');

\echo 'Messages:'
SELECT COUNT(*) AS total FROM chat_messages;

\echo '--- Seeding Complete ---'
-- =====================================
-- SAMPLE LESSON DATA
-- =====================================
-- This file contains sample lesson data for testing the lesson management features

-- Note: Make sure the users table has at least one user before running this
-- The created_by field references users(user_id)

-- =====================================
-- Beginner Level Lessons
-- =====================================

INSERT INTO lessons (title, topic, level, video_url, audio_url, text_content, vocabulary, grammar)
VALUES 
(
    'English Alphabet Basics',
    'basics',
    'beginner',
    'https://example.com/videos/alphabet.mp4',
    'https://example.com/audio/alphabet.mp3',
    'Welcome to English! The English alphabet has 26 letters: A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z. Practice saying each letter clearly.',
    '["alphabet", "letter", "vowel", "consonant", "pronunciation"]'::jsonb,
    '["There are 26 letters in English", "5 vowels: A, E, I, O, U", "21 consonants"]'::jsonb
),

(
    'Basic Greetings',
    'conversation',
    'beginner',
    'https://example.com/videos/greetings.mp4',
    'https://example.com/audio/greetings.mp3',
    'Learning how to greet people is essential. Common greetings include: Hello, Hi, Good morning, Good afternoon, Good evening. To ask how someone is doing, say "How are you?" and respond with "I am fine, thank you."',
    '["hello", "hi", "good morning", "good afternoon", "good evening", "how are you", "thank you"]'::jsonb,
    '["Use Good morning before noon", "Use Good afternoon from noon to 6pm", "Use Good evening after 6pm"]'::jsonb
),

(
    'Simple Present Tense',
    'grammar',
    'beginner',
    'https://example.com/videos/present-tense.mp4',
    'https://example.com/audio/present-tense.mp3',
    'The simple present tense describes habits, general facts, and routines. Form: Subject + Verb (base form). Examples: I eat breakfast. She reads books. They play soccer.',
    '["eat", "drink", "read", "write", "play", "study", "work", "sleep"]'::jsonb,
    '["Add -s or -es for he/she/it", "I/You/We/They + base verb", "He/She/It + verb-s", "Example: I play, He plays"]'::jsonb
),

(
    'Common Nouns',
    'vocabulary',
    'beginner',
    'https://example.com/videos/nouns.mp4',
    'https://example.com/audio/nouns.mp3',
    'Nouns are words that name people, places, things, or ideas. Common nouns include everyday objects you see around you.',
    '["apple", "book", "cat", "dog", "house", "car", "tree", "water", "school", "friend"]'::jsonb,
    '["Nouns name people, places, or things", "Capitalize proper nouns", "Use a/an before singular nouns"]'::jsonb
);

-- =====================================
-- Intermediate Level Lessons
-- =====================================

INSERT INTO lessons (title, topic, level, video_url, audio_url, text_content, vocabulary, grammar)
VALUES 
(
    'Past Tense Verbs',
    'grammar',
    'intermediate',
    'https://example.com/videos/past-tense.mp4',
    'https://example.com/audio/past-tense.mp3',
    'The simple past tense describes completed actions in the past. Regular verbs add -ed: walk → walked, play → played. Irregular verbs have special forms: go → went, eat → ate, see → saw.',
    '["yesterday", "last week", "ago", "before", "then", "previously"]'::jsonb,
    '["Regular verbs: verb + ed", "Irregular verbs: special forms", "Time markers: yesterday, last week, ago"]'::jsonb
),

(
    'Making Suggestions',
    'conversation',
    'intermediate',
    'https://example.com/videos/suggestions.mp4',
    'https://example.com/audio/suggestions.mp3',
    'Learn how to make polite suggestions in English. Use phrases like: "Why don''t we...?", "How about...?", "Let''s...", "Should we...?", "What if we...?" Example: "Why don''t we go to the park?"',
    '["suggestion", "recommend", "propose", "consider", "perhaps", "maybe"]'::jsonb,
    '["Why don''t we + verb", "How about + verb-ing", "Let''s + verb", "Should we + verb"]'::jsonb
),

(
    'Business Vocabulary',
    'vocabulary',
    'intermediate',
    'https://example.com/videos/business.mp4',
    'https://example.com/audio/business.mp3',
    'Essential business vocabulary for professional settings. Learn words related to meetings, presentations, and workplace communication.',
    '["meeting", "presentation", "deadline", "project", "client", "colleague", "manager", "schedule", "report", "conference"]'::jsonb,
    '["Use formal language in business settings", "Common phrase: I am writing to...", "Professional greetings"]'::jsonb
),

(
    'Conditional Sentences',
    'grammar',
    'intermediate',
    'https://example.com/videos/conditionals.mp4',
    'https://example.com/audio/conditionals.mp3',
    'Conditional sentences express situations and their results. First conditional: If + present simple, will + verb. Example: If it rains, I will stay home. Second conditional: If + past simple, would + verb.',
    '["if", "when", "unless", "provided", "suppose", "condition"]'::jsonb,
    '["First conditional: real possibilities", "If + present, will + verb", "Second conditional: imaginary situations", "If + past, would + verb"]'::jsonb
);

-- =====================================
-- Advanced Level Lessons
-- =====================================

INSERT INTO lessons (title, topic, level, video_url, audio_url, text_content, vocabulary, grammar)
VALUES 
(
    'Advanced Academic Writing',
    'writing',
    'advanced',
    'https://example.com/videos/academic-writing.mp4',
    'https://example.com/audio/academic-writing.mp3',
    'Academic writing requires formal language, clear structure, and evidence-based arguments. Key elements include: thesis statement, topic sentences, supporting evidence, citation of sources, and a strong conclusion.',
    '["thesis", "argument", "evidence", "citation", "methodology", "hypothesis", "conclusion", "analysis", "critique", "discourse"]'::jsonb,
    '["Use passive voice for objectivity", "Avoid contractions", "Third person perspective", "Formal vocabulary choices"]'::jsonb
),

(
    'Idiomatic Expressions',
    'vocabulary',
    'advanced',
    'https://example.com/videos/idioms.mp4',
    'https://example.com/audio/idioms.mp3',
    'Idioms are expressions that have meanings different from their literal words. Understanding idioms is crucial for natural English conversation.',
    '["piece of cake", "break the ice", "hit the nail on the head", "cost an arm and a leg", "blessing in disguise", "call it a day"]'::jsonb,
    '["Idioms cannot be translated word-by-word", "Context determines meaning", "Common in informal speech"]'::jsonb
),

(
    'Perfect Tenses and Aspects',
    'grammar',
    'advanced',
    'https://example.com/videos/perfect-tenses.mp4',
    'https://example.com/audio/perfect-tenses.mp3',
    'Master the perfect tenses: Present Perfect (have/has + past participle) for actions continuing to present or with present relevance. Past Perfect (had + past participle) for actions before another past action. Future Perfect (will have + past participle) for actions that will be completed by a future time.',
    '["already", "yet", "just", "ever", "never", "recently", "lately", "so far"]'::jsonb,
    '["Present Perfect: have/has + past participle", "Past Perfect: had + past participle", "Future Perfect: will have + past participle", "Use with time expressions"]'::jsonb
),

(
    'Debate and Persuasion',
    'speaking',
    'advanced',
    'https://example.com/videos/debate.mp4',
    'https://example.com/audio/debate.mp3',
    'Learn techniques for effective debating and persuasive speaking. Structure arguments logically, support claims with evidence, anticipate counterarguments, and use rhetorical devices effectively.',
    '["argument", "rebuttal", "evidence", "counterargument", "persuade", "convince", "rhetoric", "logic", "fallacy", "concession"]'::jsonb,
    '["State your position clearly", "Support with evidence", "Address counterarguments", "Use rhetorical questions", "Conclude with a strong statement"]'::jsonb
);

-- =====================================
-- Mixed Topics
-- =====================================

INSERT INTO lessons (title, topic, level, video_url, audio_url, text_content, vocabulary, grammar)
VALUES 
(
    'Travel English',
    'practical',
    'intermediate',
    'https://example.com/videos/travel.mp4',
    'https://example.com/audio/travel.mp3',
    'Essential English for traveling. Learn how to ask for directions, order food, book hotels, and handle common travel situations.',
    '["airport", "hotel", "reservation", "ticket", "luggage", "customs", "passport", "departure", "arrival", "destination"]'::jsonb,
    '["Polite requests: Could you...?", "Modal verbs: can, could, would", "Question formation"]'::jsonb
),

(
    'Food and Cooking Vocabulary',
    'vocabulary',
    'beginner',
    'https://example.com/videos/food.mp4',
    'https://example.com/audio/food.mp3',
    'Learn vocabulary related to food, cooking, and dining. This lesson covers common foods, cooking methods, and restaurant phrases.',
    '["bread", "rice", "vegetables", "fruit", "meat", "fish", "milk", "cheese", "breakfast", "lunch", "dinner"]'::jsonb,
    '["Countable vs uncountable nouns", "Use some/any with food", "How much/How many"]'::jsonb
);

-- =====================================
-- Verification Query
-- =====================================

-- Run this to verify lessons were inserted:
SELECT 
    lesson_id,
    title,
    topic,
    level,
    CASE 
        WHEN vocabulary IS NOT NULL THEN jsonb_array_length(vocabulary)
        ELSE 0 
    END as vocab_count,
    CASE 
        WHEN grammar IS NOT NULL THEN jsonb_array_length(grammar)
        ELSE 0 
    END as grammar_count
FROM lessons
ORDER BY level, lesson_id;

\echo '--- Inserting Exam: have today data ---'

-- Insert the Exam linked to the first lesson found
INSERT INTO exams (lesson_id, title, type, level, question, created_by)
SELECT 
    l.lesson_id,
    'have today data',
    'mixed',
    'intermediate',
    '[
        {"text": "Describe your priority tasks for today.", "type": "essay", "answer": "", "explanation": "Subjective input regarding daily priorities."},
        {"text": "What feedback do you expect?", "type": "essay", "answer": "", "explanation": "Expectation setting."}
    ]'::jsonb,
    (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1)
FROM lessons l
LIMIT 1;

-- Insert a pending result for this exam (Needs Feedback) for 'student1'
INSERT INTO results (user_id, target_type, target_id, score, user_answer, feedback, status)
VALUES (
    (SELECT user_id FROM users WHERE username = 'student1' LIMIT 1),
    'exam',
    (SELECT exam_id FROM exams WHERE title = 'have today data' ORDER BY exam_id DESC LIMIT 1),
    NULL, -- Score is null or 0 for pending
    'My priority today is to fix the exam seeding.^I expect quick feedback.',
    NULL, -- No feedback yet
    'pending'
);

\echo '--- Inserted "have today data" exam and pending result ---'

\echo '--- Inserting Exams with prefix 12_15_25_ ---'

-- 1. Multiple Choice Exam
INSERT INTO exams (lesson_id, title, type, level, question, created_by)
SELECT 
    l.lesson_id,
    '12_15_25_Morning_Quiz',
    'multiple_choice',
    'beginner',
    '[
        {"text": "What is the date today?", "type": "multiple_choice", "options": ["12/14", "12/15", "12/16"], "answer": "12/15", "explanation": "Today is the 15th."},
        {"text": "Select the correct year.", "type": "multiple_choice", "options": ["2024", "2025", "2026"], "answer": "2025", "explanation": "Current simulated year."}
    ]'::jsonb,
    (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1)
FROM lessons l
LIMIT 1;

-- 2. Mixed Exam (Subjective - for feedback testing)
INSERT INTO exams (lesson_id, title, type, level, question, created_by)
SELECT 
    l.lesson_id,
    '12_15_25_Grammar_Check',
    'mixed',
    'intermediate',
    '[
        {"text": "Write a sentence using the word: Serendipity.", "type": "essay", "answer": "", "explanation": "Creative writing."},
        {"text": "Correct this sentence: She don''t like apples.", "type": "rewrite_sentence", "answer": "She doesn''t like apples.", "explanation": "Subject-verb agreement."}
    ]'::jsonb,
    (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1)
FROM lessons l
LIMIT 1;

-- 3. Speaking Exam
INSERT INTO exams (lesson_id, title, type, level, question, created_by)
SELECT 
    l.lesson_id,
    '12_15_25_Speaking_Test',
    'mixed',
    'advanced',
    '[
        {"text": "Read the following paragraph aloud.", "type": "speaking", "answer": "", "explanation": "Pronunciation check."}
    ]'::jsonb,
    (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1)
FROM lessons l
LIMIT 1;

-- Optional: Insert a pending result for the Grammar Check exam to aid testing
INSERT INTO results (user_id, target_type, target_id, score, user_answer, feedback, status)
VALUES (
    (SELECT user_id FROM users WHERE username = 'student1' LIMIT 1),
    'exam',
    (SELECT exam_id FROM exams WHERE title = '12_15_25_Grammar_Check' ORDER BY exam_id DESC LIMIT 1),
    NULL, 
    'Serendipity is a happy accident.^She does not like apples.',
    NULL, 
    'pending'
);

\echo '--- Inserted 3 exams with prefix 12_15_25_ and 1 pending result ---'

\echo '--- Inserting Exam: have today data ---'

-- Insert the Exam linked to the first lesson found
INSERT INTO exams (lesson_id, title, type, level, question, created_by)
SELECT 
    l.lesson_id,
    'have today data',
    'mixed',
    'intermediate',
    '[
        {"text": "Describe your priority tasks for today.", "type": "essay", "answer": "", "explanation": "Subjective input regarding daily priorities."},
        {"text": "What feedback do you expect?", "type": "essay", "answer": "", "explanation": "Expectation setting."}
    ]'::jsonb,
    (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1)
FROM lessons l
LIMIT 1;

-- Insert a pending result for this exam (Needs Feedback) for 'student1'
INSERT INTO results (user_id, target_type, target_id, score, user_answer, feedback, status)
VALUES (
    (SELECT user_id FROM users WHERE username = 'student1' LIMIT 1),
    'exam',
    (SELECT exam_id FROM exams WHERE title = 'have today data' ORDER BY exam_id DESC LIMIT 1),
    NULL, -- Score is null or 0 for pending
    'My priority today is to fix the exam seeding.^I expect quick feedback.',
    NULL, -- No feedback yet
    'pending'
);

\echo '--- Inserted "have today data" exam and pending result ---'

\echo '--- Inserting Exams with prefix 12_15_25_ ---'

-- 1. Multiple Choice Exam
INSERT INTO exams (lesson_id, title, type, level, question, created_by)
SELECT 
    l.lesson_id,
    '12_15_25_Morning_Quiz',
    'multiple_choice',
    'beginner',
    '[
        {"text": "What is the date today?", "type": "multiple_choice", "options": ["12/14", "12/15", "12/16"], "answer": "12/15", "explanation": "Today is the 15th."},
        {"text": "Select the correct year.", "type": "multiple_choice", "options": ["2024", "2025", "2026"], "answer": "2025", "explanation": "Current simulated year."}
    ]'::jsonb,
    (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1)
FROM lessons l
LIMIT 1;

-- 2. Mixed Exam (Subjective - for feedback testing)
INSERT INTO exams (lesson_id, title, type, level, question, created_by)
SELECT 
    l.lesson_id,
    '12_15_25_Grammar_Check',
    'mixed',
    'intermediate',
    '[
        {"text": "Write a sentence using the word: Serendipity.", "type": "essay", "answer": "", "explanation": "Creative writing."},
        {"text": "Correct this sentence: She don''t like apples.", "type": "rewrite_sentence", "answer": "She doesn''t like apples.", "explanation": "Subject-verb agreement."}
    ]'::jsonb,
    (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1)
FROM lessons l
LIMIT 1;

-- 3. Speaking Exam
INSERT INTO exams (lesson_id, title, type, level, question, created_by)
SELECT 
    l.lesson_id,
    '12_15_25_Speaking_Test',
    'mixed',
    'advanced',
    '[
        {"text": "Read the following paragraph aloud.", "type": "speaking", "answer": "", "explanation": "Pronunciation check."}
    ]'::jsonb,
    (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1)
FROM lessons l
LIMIT 1;

-- Optional: Insert a pending result for the Grammar Check exam to aid testing
INSERT INTO results (user_id, target_type, target_id, score, user_answer, feedback, status)
VALUES (
    (SELECT user_id FROM users WHERE username = 'student1' LIMIT 1),
    'exam',
    (SELECT exam_id FROM exams WHERE title = '12_15_25_Grammar_Check' ORDER BY exam_id DESC LIMIT 1),
    NULL, 
    'Serendipity is a happy accident.^She does not like apples.',
    NULL, 
    'pending'
);

\echo '--- Inserted 3 exams with prefix 12_15_25_ and 1 pending result ---'

\echo '--- Inserting Additional Grammar Check Exams (12_15_25_) ---'

-- Grammar Check 2
INSERT INTO exams (lesson_id, title, type, level, question, created_by)
SELECT 
    l.lesson_id,
    '12_15_25_Grammar_Check_2',
    'mixed',
    'intermediate',
    '[
        {"text": "Correct: He go to school everyday.", "type": "rewrite_sentence", "answer": "He goes to school everyday.", "explanation": "Third person singular."},
        {"text": "Describe your morning routine.", "type": "essay", "answer": "", "explanation": "Narrative writing."}
    ]'::jsonb,
    (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1)
FROM lessons l
LIMIT 1;

-- Grammar Check 3
INSERT INTO exams (lesson_id, title, type, level, question, created_by)
SELECT 
    l.lesson_id,
    '12_15_25_Grammar_Check_3',
    'mixed',
    'intermediate',
    '[
        {"text": "Correct: I have seen him yesterday.", "type": "rewrite_sentence", "answer": "I saw him yesterday.", "explanation": "Specific past time requires Past Simple."},
        {"text": "Write a short email to a friend.", "type": "essay", "answer": "", "explanation": "Informal writing."}
    ]'::jsonb,
    (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1)
FROM lessons l
LIMIT 1;

-- Grammar Check 4
INSERT INTO exams (lesson_id, title, type, level, question, created_by)
SELECT 
    l.lesson_id,
    '12_15_25_Grammar_Check_4',
    'mixed',
    'advanced',
    '[
        {"text": "Correct: If I was you, I would go.", "type": "rewrite_sentence", "answer": "If I were you, I would go.", "explanation": "Subjunctive mood."},
        {"text": "Explain the impact of technology on society.", "type": "essay", "answer": "", "explanation": "Argumentative writing."}
    ]'::jsonb,
    (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1)
FROM lessons l
LIMIT 1;

-- Grammar Check 5
INSERT INTO exams (lesson_id, title, type, level, question, created_by)
SELECT 
    l.lesson_id,
    '12_15_25_Grammar_Check_5',
    'mixed',
    'advanced',
    '[
        {"text": "Correct: The data are valid.", "type": "rewrite_sentence", "answer": "The data is valid. (or are, dependent on style guide)", "explanation": "Collective nouns usage."},
        {"text": "Summarize the plot of your favorite movie.", "type": "essay", "answer": "", "explanation": "Summary skills."}
    ]'::jsonb,
    (SELECT user_id FROM users WHERE role = 'teacher' OR role = 'admin' LIMIT 1)
FROM lessons l
LIMIT 1;

\echo '--- Inserted 4 more Grammar Check Exams ---'

\echo '--- Inserting Game Items (3 per type: beginner/intermediate/advanced) ---'

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
 ]'::jsonb, 1),

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
 ]'::jsonb, 1),

('word_match', 'advanced',
 '[
    {"word_pair": ["Ubiquitous", "Everywhere"]},
    {"word_pair": ["Mellifluous", "Sweet-sounding"]},
    {"word_pair": ["Obfuscate", "Confuse"]},
    {"word_pair": ["Pernicious", "Harmful"]}
 ]'::jsonb, 1);

-- Image Match Games (Using local paths)
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
 ]'::jsonb, 1),

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
 ]'::jsonb, 1),

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
 ]'::jsonb, 1);

\echo '--- Inserted 9 game items (3 per type) ---'
