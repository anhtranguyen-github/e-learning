
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
