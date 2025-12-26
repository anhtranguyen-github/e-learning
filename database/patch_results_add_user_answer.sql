-- Add missing columns to results table (run once).

ALTER TABLE results ADD COLUMN user_answer TEXT;
ALTER TABLE results ADD COLUMN status VARCHAR(20);
ALTER TABLE results ADD COLUMN grading_details JSONB;

UPDATE results
SET status = 'graded'
WHERE status IS NULL;
