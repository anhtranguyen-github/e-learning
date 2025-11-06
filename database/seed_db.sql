-- =====================================
-- DATABASE SEED SCRIPT
-- =====================================

INSERT INTO users (username, password_hash, full_name, role, level)
VALUES
('admin1', 'admin_hash', 'Admin User', 'admin', 'advanced'),
('teacher1', 'teacher_hash', 'John Teacher', 'teacher', 'advanced'),
('student1', 'student_hash', 'Alice Student', 'student', 'beginner');

INSERT INTO lessons (title, topic, level, video_url, audio_url, text_content, created_by)
VALUES
('Basic Greetings', 'Communication', 'beginner',
 'https://example.com/video1', 'https://example.com/audio1',
 'Hello! How are you?', 2);

INSERT INTO exercises (lesson_id, type, question, answer, created_by)
VALUES
(1, 'rewrite_sentence', 'Rewrite: I am happy.', 'I am glad.', 2);

INSERT INTO exams (lesson_id, type, question, created_by)
VALUES
(1, 'multiple_choice',
 '[{"q":"What is the greeting?","options":["Hi","Bye"],"answer":"Hi"}]', 2);

INSERT INTO results (user_id, target_type, target_id, score, feedback)
VALUES
(3, 'exam', 1, 95.5, 'Good job!');

INSERT INTO game_items (type, question, level, created_by)
VALUES
('word_match', '[{"a":"cat","b":"con mèo"},{"a":"dog","b":"con chó"}]', 'beginner', 1);

INSERT INTO server_sessions (session_id, user_id, client_fd, expires_at)
VALUES
('sess-abc123', 3, 12, NOW() + INTERVAL '2 hours');
