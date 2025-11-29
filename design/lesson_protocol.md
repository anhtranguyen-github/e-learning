# Lesson Protocol Design

## Overview
The Lesson protocol manages the retrieval of lesson lists and detailed lesson content.

## Messages

| Message Name | Code | Description | Payload Structure |
| :--- | :--- | :--- | :--- |
| `LESSON_LIST_REQUEST` | 120 | Request a list of lessons filtered by topic/level. | `sessionToken;topic;level` |
| `LESSON_LIST_SUCCESS` | 121 | Returns a list of lesson metadata. | List of `id|title|topic|level`, separated by `;` |
| `LESSON_LIST_FAILURE` | 122 | Failed to retrieve lesson list. | `error_message` |
| `STUDY_LESSON_REQUEST` | 130 | Request full details for a specific lesson. | `sessionToken;lessonId;lessonType` |
| `STUDY_LESSON_SUCCESS` | 131 | Returns full lesson content. | `LessonDTO` serialized string |
| `STUDY_LESSON_FAILURE` | 132 | Failed to retrieve lesson details. | `error_message` |

## Payload Definitions

### LessonListRequest
- **Fields**: `sessionToken`, `topic`, `level`
- **Serialization**: `sessionToken;topic;level`

### StudyLessonRequest
- **Fields**: `sessionToken`, `lessonId`, `lessonType`
- **Serialization**: `sessionToken;lessonId;lessonType`

### LessonMetadataDTO
- **Fields**: `id`, `title`, `topic`, `level`
- **Serialization**: `id|title|topic|level`

### LessonDTO
- **Fields**: `id`, `title`, `topic`, `level`, `videoUrl`, `audioUrl`, `textContent`, `vocabulary` (list), `grammar` (list)
- **Serialization**: `id|title|topic|level|videoUrl|audioUrl|textContent|vocab1,vocab2|grammar1,grammar2`

## Example Flow

### Fetching Lesson List
1.  **Client** sends `LESSON_LIST_REQUEST`:
    ```
    Code: 120
    Payload: "token123;Science;Beginner"
    ```
2.  **Server** responds with `LESSON_LIST_SUCCESS`:
    ```
    Code: 121
    Payload: "1|Intro to Physics|Science|Beginner;2|Chemistry Basics|Science|Beginner"
    ```

### Fetching Lesson Details
1.  **Client** sends `STUDY_LESSON_REQUEST`:
    ```
    Code: 130
    Payload: "token123;1;video"
    ```
2.  **Server** responds with `STUDY_LESSON_SUCCESS`:
    ```
    Code: 131
    Payload: "1|Intro to Physics|Science|Beginner|http://vid.url|http://aud.url|Content...|atom,molecule|noun,verb"
    ```
