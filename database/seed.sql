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
