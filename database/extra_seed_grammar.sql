
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
