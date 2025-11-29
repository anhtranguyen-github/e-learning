-- =====================================
-- DATABASE INITIALIZATION SCRIPT
-- =====================================

CREATE TABLE IF NOT EXISTS users (
    user_id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    full_name VARCHAR(100),
    role VARCHAR(50) CHECK (role IN ('admin', 'teacher', 'student')) NOT NULL,
    level VARCHAR(50) CHECK (level IN ('beginner', 'intermediate', 'advanced')),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS lessons (
    lesson_id SERIAL PRIMARY KEY,
    title VARCHAR(100) NOT NULL,
    topic VARCHAR(100),
    level VARCHAR(50) CHECK (level IN ('beginner', 'intermediate', 'advanced')),
    video_url TEXT,
    audio_url TEXT,
    text_content TEXT,
    vocabulary JSONB,
    grammar JSONB,
    created_by INT REFERENCES users(user_id) ON DELETE SET NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS exercises (
    exercise_id SERIAL PRIMARY KEY,
    lesson_id INT REFERENCES lessons(lesson_id) ON DELETE CASCADE,
    title VARCHAR(100) NOT NULL,
    type VARCHAR(50) CHECK (type IN ('rewrite_sentence', 'essay', 'speaking', 'multiple_choice', 'fill_in_blank', 'order')),
    level VARCHAR(50) CHECK (level IN ('beginner', 'intermediate', 'advanced')),
    options JSONB,
    question TEXT, -- Made nullable as we might use 'questions' instead
    questions JSONB, -- New column for multiple questions
    answer TEXT,
    media_url TEXT,
    explanation TEXT,
    created_by INT REFERENCES users(user_id) ON DELETE SET NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS exams (
    exam_id SERIAL PRIMARY KEY,
    lesson_id INT REFERENCES lessons(lesson_id) ON DELETE CASCADE,
    title VARCHAR(100) NOT NULL,
    type VARCHAR(50) CHECK (type IN ('multiple_choice', 'fill_blank', 'order_sentence', 'mixed')),
    level VARCHAR(50) CHECK (level IN ('beginner', 'intermediate', 'advanced')),
    question JSONB,
    created_by INT REFERENCES users(user_id) ON DELETE SET NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS results (
    result_id SERIAL PRIMARY KEY,
    user_id INT REFERENCES users(user_id) ON DELETE CASCADE,
    target_type VARCHAR(50) CHECK (target_type IN ('exam','exercise')),
    target_id INT NOT NULL,
    score NUMERIC(5,2),
    user_answer TEXT,
    feedback TEXT,
    graded_by VARCHAR(50) DEFAULT 'machine',
    graded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    submitted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    status VARCHAR(20) CHECK (status IN ('pending', 'graded')) DEFAULT 'graded'
);

CREATE TABLE IF NOT EXISTS game_items (
    game_id SERIAL PRIMARY KEY,
    type VARCHAR(50) CHECK (type IN ('word_match', 'sentence_match', 'image_match')),
    question JSONB,
    level VARCHAR(50) CHECK (level IN ('beginner', 'intermediate', 'advanced')),
    created_by INT REFERENCES users(user_id) ON DELETE SET NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- ==============================
-- SERVER_SESSIONS TABLE
-- ==============================
CREATE TABLE IF NOT EXISTS server_sessions (
    session_id      VARCHAR(255) PRIMARY KEY,               -- Unique session token
    user_id         INTEGER NOT NULL,                       -- Link to users table
    client_fd       INTEGER,                                -- File descriptor (optional)
    last_active     TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    active          BOOLEAN NOT NULL DEFAULT TRUE,
    created_at      TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    expires_at      TIMESTAMP,                              -- Optional expiration
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);
-- Create chat_messages table
CREATE TABLE IF NOT EXISTS chat_messages (
    id SERIAL PRIMARY KEY,
    sender_id INTEGER NOT NULL,
    receiver_id INTEGER NOT NULL,
    content TEXT NOT NULL,
    message_type VARCHAR(20) CHECK (message_type IN ('TEXT', 'AUDIO')) DEFAULT 'TEXT',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    is_read BOOLEAN NOT NULL DEFAULT FALSE,
    FOREIGN KEY (sender_id) REFERENCES users(user_id),
    FOREIGN KEY (receiver_id) REFERENCES users(user_id)
);
