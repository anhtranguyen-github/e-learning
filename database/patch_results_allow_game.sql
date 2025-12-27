-- Allow specific game target types in results.

ALTER TABLE results DROP CONSTRAINT IF EXISTS results_target_type_check;

UPDATE results r
SET target_type = gi.type || '_game'
FROM game_items gi
WHERE r.target_type = 'game'
  AND r.target_id = gi.game_id;

ALTER TABLE results
  ADD CONSTRAINT results_target_type_check
  CHECK (target_type IN ('exam', 'exercise', 'word_match_game', 'sentence_match_game', 'image_match_game'));
