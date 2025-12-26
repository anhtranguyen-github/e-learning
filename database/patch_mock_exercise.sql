-- Add or fix a mock essay exercise for testing submissions.

INSERT INTO exercises (exercise_id, lesson_id, title, type, level, question, answer, created_by)
SELECT
    24,
    1,
    'Exercise 24 (Mock Essay)',
    'essay',
    'intermediate',
    'Write a short paragraph about your favorite hobby.',
    '',
    1
WHERE NOT EXISTS (SELECT 1 FROM exercises WHERE exercise_id = 24);

UPDATE exercises
SET question = 'Write a short paragraph about your favorite hobby.',
    answer = ''
WHERE exercise_id = 24
  AND (question IS NULL OR question = '');

SELECT setval(
    pg_get_serial_sequence('exercises', 'exercise_id'),
    GREATEST((SELECT COALESCE(MAX(exercise_id), 1) FROM exercises), 24)
);
