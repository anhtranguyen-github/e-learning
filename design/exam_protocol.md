# Exam Protocol Design

## Overview
The Exam protocol manages the retrieval of exam lists and specific exam content.

## Messages

| Message Name | Code | Description | Payload Structure |
| :--- | :--- | :--- | :--- |
| `EXAM_LIST_REQUEST` | 270 | Request list of exams. | `sessionToken;type;level;lessonId` |
| `EXAM_LIST_SUCCESS` | 271 | Returns list of exam metadata. | List of `ExamMetadataDTO` |
| `EXAM_LIST_FAILURE` | 272 | Failed to list exams. | `error_message` |
| `EXAM_REQUEST` | 275 | Request specific exam details. | `sessionToken;examId` |
| `EXAM_SUCCESS` | 276 | Returns full exam content. | `ExamDTO` serialized string |
| `EXAM_FAILURE` | 277 | Failed to retrieve exam. | `error_message` |

## Payload Definitions

### ExamListRequest
- **Fields**: `sessionToken`, `type`, `level`, `lessonId`
- **Serialization**: `sessionToken;type;level;lessonId`

### ExamRequest
- **Fields**: `sessionToken`, `examId`
- **Serialization**: `sessionToken;examId`

### ExamMetadataDTO
- **Fields**: `id`, `lessonId`, `title`, `type`, `level`
- **Serialization**: `id|lessonId|title|type|level`

### ExamDTO
- **Fields**: `id`, `lessonId`, `title`, `type`, `level`, `questions` (list)
- **Serialization**: `id|lessonId|title|type|level|q1^q2^q3`

## Example Flow

### Fetching Exam List
1.  **Client** sends `EXAM_LIST_REQUEST`:
    ```
    Code: 270
    Payload: "token123;Midterm;Intermediate;5"
    ```
2.  **Server** responds with `EXAM_LIST_SUCCESS`:
    ```
    Code: 271
    Payload: "10|5|Midterm Exam|Midterm|Intermediate;11|5|Final Exam|Final|Intermediate"
    ```

### Fetching Exam Content
1.  **Client** sends `EXAM_REQUEST`:
    ```
    Code: 275
    Payload: "token123;10"
    ```
2.  **Server** responds with `EXAM_SUCCESS`:
    ```
    Code: 276
    Payload: "10|5|Midterm Exam|Midterm|Intermediate|Question1?^Question2?"
    ```
