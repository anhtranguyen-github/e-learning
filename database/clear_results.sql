
-- Remove results for student1 on the specific new exams so they can be taken again.

DELETE FROM results 
WHERE user_id = (SELECT user_id FROM users WHERE username = 'student1')
  AND target_type = 'exam'
  AND target_id IN (
      SELECT exam_id FROM exams 
      WHERE title = 'have today data' 
         OR title LIKE '12_15_25_%'
  );

\echo '--- Cleared results for student1 on new exams. They are now "doable" (takeable) again. ---'
