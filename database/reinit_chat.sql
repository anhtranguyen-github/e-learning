-- Drop existing tables to ensure clean schema update
DROP TABLE IF EXISTS chat_messages CASCADE;
DROP TABLE IF EXISTS call_logs CASCADE;

-- Create call_logs table for optional analytics
CREATE TABLE IF NOT EXISTS call_logs (
    call_id SERIAL PRIMARY KEY,
    caller_id INTEGER NOT NULL REFERENCES users(user_id),
    receiver_id INTEGER NOT NULL REFERENCES users(user_id),
    start_time TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    end_time TIMESTAMPTZ,
    status VARCHAR(20) CHECK (status IN ('COMPLETED', 'MISSED', 'BUSY', 'DECLINED', 'FAILED')),
    duration INTEGER -- duration in seconds
);

-- Recreate chat_messages table with updated message_type constraint
CREATE TABLE IF NOT EXISTS chat_messages (
    id SERIAL PRIMARY KEY,
    sender_id INTEGER NOT NULL,
    receiver_id INTEGER NOT NULL,
    content TEXT NOT NULL,
    -- Expanded message_type to include SYSTEM
    message_type VARCHAR(20) CHECK (message_type IN ('TEXT', 'AUDIO', 'SYSTEM')) DEFAULT 'TEXT',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    is_read BOOLEAN NOT NULL DEFAULT FALSE,
    FOREIGN KEY (sender_id) REFERENCES users(user_id),
    FOREIGN KEY (receiver_id) REFERENCES users(user_id)
);
