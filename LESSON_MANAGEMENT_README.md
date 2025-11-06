# Lesson Management Feature - Implementation Guide

## Overview

This document describes the complete implementation of the lesson management features for the TCP client-server learning application. The system supports viewing lesson lists with filters and studying individual lessons by content type.

---

## Architecture

### Message Flow

```
Client                                Server
  |                                      |
  |-- LESSON_LIST_REQUEST -------------->|
  |   (token, optional filters)          |
  |                                      |
  |   Validate session                   |
  |   Load lessons from DB               |
  |   Apply filters                      |
  |   Serialize lesson list              |
  |                                      |
  |<-- LESSON_LIST_SUCCESS --------------|
  |   (lesson metadata list)             |
  |                                      |
  |-- STUDY_LESSON_REQUEST ------------->|
  |   (token, lesson_id, type)           |
  |                                      |
  |   Validate session                   |
  |   Load lesson from DB                |
  |   Serialize requested content        |
  |                                      |
  |<-- STUDY_LESSON_SUCCESS -------------|
  |   (lesson content)                   |
```

---

## Components Implemented

### 1. Protocol Layer (`include/common/protocol.h`)

**New Message Codes:**
- `LESSON_LIST_REQUEST = 110`
- `LESSON_LIST_SUCCESS = 111`
- `LESSON_LIST_FAILURE = 113`
- `STUDY_LESSON_REQUEST = 120`
- `STUDY_LESSON_SUCCESS = 121`
- `STUDY_LESSON_FAILURE = 123`

### 2. Server Components

#### **`include/server/lesson_loader.h` & `src/server/lesson_loader.cpp`**

**Classes:**
- **`Lesson`**: Represents a single lesson with all content fields
  - Fields: `lesson_id`, `title`, `topic`, `level`, `video_url`, `audio_url`, `text_content`, `vocabulary`, `grammar`
  - Methods:
    - `serializeForNetwork(LessonType)`: Serializes only requested content type
    - `serializeMetadata()`: Returns metadata for lesson list display

- **`LessonList`**: Container for multiple lessons
  - Methods:
    - `filter(topic, level)`: Filters lessons by criteria
    - `serializeForNetwork()`: Serializes all lesson metadata

- **`LessonLoader`**: Database loader class
  - Methods:
    - `loadAllLessons()`: Loads all lessons from PostgreSQL
    - `loadLessonById(id)`: Loads specific lesson with full content
    - `loadLessonsByFilter(topic, level)`: Loads filtered lessons
  - Helper:
    - `parseJsonArray()`: Parses PostgreSQL JSONB fields for vocabulary/grammar

**Database Queries:**
```sql
-- Load all lessons
SELECT lesson_id, title, topic, level, video_url, audio_url, 
       text_content, vocabulary::text, grammar::text 
FROM lessons 
ORDER BY lesson_id;

-- Load specific lesson
SELECT ... FROM lessons WHERE lesson_id = <id>;

-- Load filtered lessons
SELECT ... FROM lessons WHERE topic = '<topic>' AND level = '<level>';
```

#### **`include/server/lesson_handler.h` & `src/server/lesson_handler.cpp`**

**Class: `LessonHandler`**
- Handles lesson-related messages from clients
- Validates session tokens before processing requests
- Methods:
  - `handleLessonListRequest()`: Processes lesson list requests
  - `handleStudyLessonRequest()`: Processes study lesson requests
  - `parseLessonType()`: Converts string to LessonType enum

**Request Handlers:**

1. **LESSON_LIST_REQUEST**
   - Payload format: `<session_token>[;<topic>;<level>]`
   - Validates session
   - Loads lessons (all or filtered)
   - Returns serialized lesson list

2. **STUDY_LESSON_REQUEST**
   - Payload format: `<session_token>;<lesson_id>;<lesson_type>`
   - Validates session
   - Loads specific lesson
   - Returns only requested content type

#### **`include/server/client_handler.h` & `src/server/client_handler.cpp`**

**Integration:**
- Added `lessonHandler` member to `ClientHandler`
- Added `setLessonHandler()` method for dependency injection
- Updated `processMessage()` switch statement to route lesson messages:
  ```cpp
  case protocol::MsgCode::LESSON_LIST_REQUEST:
      lessonHandler->handleLessonListRequest(clientFd, msg);
      break;
  
  case protocol::MsgCode::STUDY_LESSON_REQUEST:
      lessonHandler->handleStudyLessonRequest(clientFd, msg);
      break;
  ```

### 3. Client Components

#### **`include/client/network.h` & `src/client/network.cpp`**

**New Methods:**
- `requestLessonList(topic, level)`: Sends LESSON_LIST_REQUEST
  - Parameters: Optional topic and level filters (empty string = no filter)
  - Builds payload with session token and filters
  
- `requestStudyLesson(lessonId, lessonType)`: Sends STUDY_LESSON_REQUEST
  - Parameters: Lesson ID and content type (video/audio/text/vocabulary/grammar/full)
  - Builds payload with session token, ID, and type

#### **`include/client/ui.h` & `src/client/ui.cpp`**

**Updated Menu:**
```
1. View Lesson List
2. Study Lesson
3. View Status
4. Logout
5. Exit
```

**New Methods:**
- `handleViewLessons()`: Interactive lesson list viewer
  - Prompts for topic/level filters
  - Sends request and displays response
  
- `handleStudyLesson()`: Interactive lesson content viewer
  - Prompts for lesson ID and content type
  - Sends request and displays formatted content
  
- `parseLessonList()`: Parses and displays lesson metadata
  - Format: `<count>;<id>|<title>|<topic>|<level>;...`
  
- `displayLessonContent()`: Displays lesson content by type
  - Handles different formats: VIDEO, AUDIO, TEXT, VOCABULARY, GRAMMAR, FULL

---

## Payload Formats

### LESSON_LIST_REQUEST
```
<session_token>
<session_token>;<topic>;
<session_token>;;<level>
<session_token>;<topic>;<level>
```

### LESSON_LIST_SUCCESS
```
<count>;<lesson1_meta>;<lesson2_meta>;...

Where lesson_meta = <id>|<title>|<topic>|<level>
Example: 3;1|Introduction to English|grammar|beginner;2|Advanced Speaking|speaking|advanced;...
```

### STUDY_LESSON_REQUEST
```
<session_token>;<lesson_id>;<lesson_type>

Example: abc123def456;5;vocabulary
```

### STUDY_LESSON_SUCCESS
```
VIDEO|<video_url>
AUDIO|<audio_url>
TEXT|<text_content>
VOCABULARY|<word1>,<word2>,<word3>,...
GRAMMAR|<rule1>,<rule2>,<rule3>,...
FULL|ID:<id>|TITLE:<title>|TOPIC:<topic>|LEVEL:<level>|VIDEO:<url>|AUDIO:<url>|TEXT:<text>|VOCAB:<words>|GRAMMAR:<rules>
```

---

## Server Setup

### 1. Initialize LessonLoader and LessonHandler

In your server main file (e.g., `src/server/server.cpp`):

```cpp
#include "server/lesson_loader.h"
#include "server/lesson_handler.h"

// Create shared instances
auto database = std::make_shared<Database>(connectionString);
auto sessionManager = std::make_shared<SessionManager>(database);
auto userManager = std::make_shared<UserManager>(database);

// Create lesson components
auto lessonLoader = std::make_shared<LessonLoader>(database);
auto lessonHandler = std::make_shared<LessonHandler>(sessionManager, lessonLoader);

// Create client handler and inject lesson handler
auto clientHandler = std::make_shared<ClientHandler>(sessionManager, userManager);
clientHandler->setLessonHandler(lessonHandler);
```

### 2. Database Requirements

Ensure the `lessons` table exists with the following structure:

```sql
CREATE TABLE IF NOT EXISTS lessons (
    lesson_id SERIAL PRIMARY KEY,
    title VARCHAR(100) NOT NULL,
    topic VARCHAR(100),
    level VARCHAR(10) CHECK (level IN ('beginner', 'intermediate', 'advanced')),
    video_url TEXT,
    audio_url TEXT,
    text_content TEXT,
    vocabulary JSONB,  -- JSON array: ["word1", "word2", ...]
    grammar JSONB,     -- JSON array: ["rule1", "rule2", ...]
    created_by INT REFERENCES users(user_id) ON DELETE SET NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### 3. Sample Data

Insert sample lessons:

```sql
INSERT INTO lessons (title, topic, level, video_url, audio_url, text_content, vocabulary, grammar)
VALUES 
('English Basics', 'grammar', 'beginner', 
 'https://example.com/video1.mp4', 
 'https://example.com/audio1.mp3',
 'This lesson covers basic English grammar including nouns, verbs, and adjectives.',
 '["apple", "book", "cat", "dog", "house"]'::jsonb,
 '["Subject + Verb + Object", "Adjectives before nouns"]'::jsonb),

('Intermediate Conversation', 'speaking', 'intermediate',
 'https://example.com/video2.mp4',
 'https://example.com/audio2.mp3',
 'Practice everyday conversations with common phrases and expressions.',
 '["How are you?", "Nice to meet you", "Thank you very much"]'::jsonb,
 '["Use present tense for habits", "Past tense for completed actions"]'::jsonb);
```

---

## Client Usage

### View Lesson List
1. Login to the application
2. Select option `1. View Lesson List`
3. Enter optional filters (or leave empty for all lessons)
4. View the displayed lesson list with IDs, titles, topics, and levels

### Study a Lesson
1. Login to the application
2. Select option `2. Study Lesson`
3. Enter the lesson ID (from the lesson list)
4. Choose content type:
   - `video`: Returns video URL only
   - `audio`: Returns audio URL only
   - `text`: Returns text content only
   - `vocabulary`: Returns vocabulary words
   - `grammar`: Returns grammar rules
   - `full`: Returns all content
5. View the displayed content

---

## Features

### Session Validation
- All lesson requests require valid session tokens
- Tokens are validated before processing
- Session activity is updated on each request (for heartbeat timeout tracking)

### Content Filtering
- Filter lessons by topic (e.g., "grammar", "speaking")
- Filter lessons by level ("beginner", "intermediate", "advanced")
- Combine filters or leave empty for all lessons

### Selective Content Loading
- Request only the content type you need
- Reduces network bandwidth and improves performance
- Server loads full lesson but serializes only requested content

### Error Handling
- Invalid session: Returns `LESSON_LIST_FAILURE` or `STUDY_LESSON_FAILURE`
- Lesson not found: Returns failure message
- Database errors: Logged on server, failure message sent to client

### Multiplexing & Heartbeat Compatibility
- Lesson requests integrate seamlessly with existing heartbeat system
- Session validation ensures user is authenticated
- Non-blocking socket operations maintained throughout

---

## Compilation

Add the new source files to your Makefile:

```makefile
SERVER_OBJS += src/server/lesson_loader.o src/server/lesson_handler.o
```

Required libraries:
- PostgreSQL: `-lpq`
- JsonCpp: `-ljsoncpp` (for parsing JSONB fields)

---

## Testing

### Test Lesson List Request
```bash
# As client, after login:
1. View Lesson List
   - Leave filters empty to see all lessons
   - Try filtering by topic: "grammar"
   - Try filtering by level: "beginner"
```

### Test Study Lesson Request
```bash
# As client, after login:
2. Study Lesson
   - Enter lesson ID: 1
   - Enter type: text
   - Verify text content displays correctly
   
# Try different types:
   - video (should show URL)
   - vocabulary (should list words)
   - full (should show all fields)
```

### Server Logs
Monitor server logs to verify:
- Session validation occurs
- Database queries execute successfully
- Lessons are loaded and serialized
- Responses are sent correctly

---

## Architecture Benefits

1. **Modular Design**: Lesson components are separate from core server logic
2. **Session Security**: All requests validated against active sessions
3. **Efficient Filtering**: Database-level filtering for performance
4. **Flexible Content**: Request only what you need (video, audio, text, etc.)
5. **Scalability**: Easy to add new lesson types or content fields
6. **Maintainability**: Clear separation of concerns (loader, handler, client)

---

## Future Enhancements

Potential improvements:
- Pagination for large lesson lists
- Search functionality (by keyword in title/content)
- Progress tracking (which lessons completed)
- Lesson ratings and reviews
- Multimedia streaming instead of URL delivery
- Lesson recommendations based on user level
- Quiz/exercise integration with lessons

---

## Summary

The lesson management system is fully integrated with:
- ✅ Session validation and heartbeat system
- ✅ PostgreSQL database with JSONB support
- ✅ TCP multiplexed client-server communication
- ✅ Graceful error handling
- ✅ Modular, extensible architecture
- ✅ Interactive client UI
- ✅ Server-side logging

All components are production-ready and follow the existing codebase patterns.
