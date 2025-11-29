# Database Schema

## Overview
The Socker Learning Platform uses a PostgreSQL database to store user data, learning content, and progress. The schema is designed to be normalized and relational.

## Entity-Relationship Diagram

```mermaid
erDiagram
    USERS ||--o{ RESULTS : has
    USERS ||--o{ CHAT_MESSAGES : sends
    USERS ||--o{ CHAT_MESSAGES : receives
    LESSONS ||--o{ EXERCISES : contains
    LESSONS ||--o{ EXAMS : related_to
    EXERCISES ||--o{ RESULTS : generates
    EXAMS ||--o{ RESULTS : generates

    USERS {
        int user_id PK
        string username
        string password_hash
        string full_name
        string role "student, teacher, admin"
        string level
    }

    LESSONS {
        int lesson_id PK
        string title
        string topic
        string level
        string video_url
        string audio_url
        text text_content
        json vocabulary
        json grammar
    }

    EXERCISES {
        int exercise_id PK
        int lesson_id FK
        string title
        string type "multiple_choice, fill_in, etc."
        string level
        json questions
    }

    EXAMS {
        int exam_id PK
        string title
        json questions
    }

    RESULTS {
        int result_id PK
        int user_id FK
        string target_type "exercise, exam"
        int target_id
        double score
        string user_answer
        string feedback
        string status "pending, graded"
        timestamp submitted_at
        timestamp graded_at
    }

    CHAT_MESSAGES {
        int id PK
        int sender_id FK
        int receiver_id FK
        text content
        string message_type "TEXT, AUDIO"
        timestamp created_at
        boolean is_read
    }
```

## Tables Description

### 1. Users
Stores account information for all users.
-   **role**: Determines access permissions (RBAC).
-   **level**: Current proficiency level (e.g., A1, B2).

### 2. Lessons
Core learning content.
-   **vocabulary/grammar**: Stored as JSON arrays for flexibility.
-   **media**: URLs to video/audio resources.

### 3. Exercises
Practice material linked to lessons.
-   **questions**: Stored as JSON to support various question types (Multiple Choice, Fill-in-the-blank, etc.) without complex join tables.

### 4. Exams
Assessments not necessarily tied to a single lesson.
-   **questions**: JSON structure similar to exercises.

### 5. Results
Tracks student progress and submissions.
-   **Polymorphic Association**: Links to either `Exercise` or `Exam` via `target_type` and `target_id`.
-   **status**: Used for teacher grading workflow ('pending' -> 'graded').

### 6. ChatMessages
Stores communication history.
-   **message_type**: Supports text and audio messages (base64 encoded or file paths).

## Design Rationale

### JSON Columns
We use `JSONB` (or `JSON`) columns for `questions`, `vocabulary`, and `grammar`.
-   **Flexibility**: Allows different question structures (options vs text input) without changing the schema.
-   **Simplicity**: Fetches all related data in a single query, avoiding N+1 query problems.

### Polymorphic Results
Instead of separate `exam_results` and `exercise_results` tables, a single `results` table simplifies querying for a user's overall progress and "pending submissions" for teachers.
