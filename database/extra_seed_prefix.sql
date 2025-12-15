
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
