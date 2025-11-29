# Exercise Protocol Design

## Overview
The Exercise protocol handles listing exercises, retrieving specific exercise details, and submitting answers.

## Messages

### General Management
| Message Name | Code | Description | Payload Structure |
| :--- | :--- | :--- | :--- |
| `EXERCISE_LIST_REQUEST` | 150 | Request list of exercises. | `sessionToken;type;level;lessonId` |
| `EXERCISE_LIST_SUCCESS` | 151 | Returns list of exercise metadata. | List of `ExerciseMetadataDTO` |
| `EXERCISE_LIST_FAILURE` | 152 | Failed to list exercises. | `error_message` |
| `STUDY_EXERCISE_REQUEST` | 160 | Request specific exercise details. | `sessionToken;exerciseId;exerciseType` |
| `STUDY_EXERCISE_SUCCESS` | 161 | Returns full exercise content. | `ExerciseDTO` serialized string |
| `STUDY_EXERCISE_FAILURE` | 162 | Failed to retrieve exercise. | `error_message` |

### Specific Exercise Types
Range 170-222 covers specific types like `MULTIPLE_CHOICE`, `FILL_IN`, `SENTENCE_ORDER`, etc.
- **Request**: `SpecificExerciseRequest` (`sessionToken;exerciseId`)
- **Success**: `ExerciseDTO` serialized string.
- **Failure**: `error_message`.

### Submission and Results
| Message Name | Code | Description | Payload Structure |
| :--- | :--- | :--- | :--- |
| `SUBMIT_ANSWER_REQUEST` | 250 | Submit an answer for grading. | `sessionToken;targetType;targetId;answer` |
| `SUBMIT_ANSWER_SUCCESS` | 251 | Submission accepted/graded. | `ResultDTO` (score|feedback) |
| `SUBMIT_ANSWER_FAILURE` | 252 | Submission failed. | `error_message` |
| `RESULT_LIST_REQUEST` | 260 | Request past results. | `sessionToken;targetType` |
| `RESULT_LIST_SUCCESS` | 261 | Returns list of results. | List of `ResultSummaryDTO` |
| `RESULT_REQUEST` | 265 | Request specific result. | `sessionToken;targetType;targetId` |

## Payload Definitions

### ExerciseListRequest
- **Fields**: `sessionToken`, `type`, `level`, `lessonId`
- **Serialization**: `sessionToken;type;level;lessonId`

### ExerciseDTO
- **Fields**: `id`, `lessonId`, `title`, `type`, `level`, `questions` (list)
- **Serialization**: `id|lessonId|title|type|level|q1^q2^q3`

### SubmitAnswerRequest
- **Fields**: `sessionToken`, `targetType`, `targetId`, `answer`
- **Serialization**: `sessionToken;targetType;targetId;answer`

### ResultDTO
- **Fields**: `score`, `feedback`
- **Serialization**: `score|feedback`

## Example Flow

### Submitting an Answer
1.  **Client** sends `SUBMIT_ANSWER_REQUEST`:
    ```
    Code: 250
    Payload: "token123;quiz;101;Paris"
    ```
2.  **Server** grades and responds with `SUBMIT_ANSWER_SUCCESS`:
    ```
    Code: 251
    Payload: "100|Correct! Paris is the capital of France."
    ```
