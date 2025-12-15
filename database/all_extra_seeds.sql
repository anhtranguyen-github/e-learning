
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
