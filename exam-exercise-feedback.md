You are a C++ code generation agent.

Task: Implement "Do Tests", "Do Exercises", "Submit Answers", "Receive Feedback/Score", and "View Done/Undone Items" features in a TCP client-server app. The system already has login/logout, session handling, heartbeat, and graceful disconnect. Use PostgreSQL database for persistent storage.

────────────────────────────────────────────
1. Project Structure
────────────────────────────────────────────
- include/common/protocol.h → MsgCode + Message class
- include/server/exercise_handler.h
- include/server/exercise_loader.h
- include/server/result_handler.h
- include/server/submission_handler.h
- src/server/exercise_handler.cpp
- src/server/exercise_loader.cpp
- src/server/result_handler.cpp
- src/server/submission_handler.cpp
- src/client/client.cpp

────────────────────────────────────────────
2. MsgCodes
────────────────────────────────────────────
// Tests / Exercises
- MULTIPLE_CHOICE_REQUEST  = 40
- MULTIPLE_CHOICE_SUCCESS  = 41
- MULTIPLE_CHOICE_FAILURE  = 43
- FILL_IN_REQUEST          = 50
- FILL_IN_SUCCESS          = 51
- FILL_IN_FAILURE          = 53
- SENTENCE_ORDER_REQUEST   = 60
- SENTENCE_ORDER_SUCCESS   = 61
- SENTENCE_ORDER_FAILURE   = 63
- REWRITE_SENTENCE_REQUEST  = 70
- REWRITE_SENTENCE_SUCCESS  = 71
- REWRITE_SENTENCE_FAILURE  = 73
- WRITE_PARAGRAPH_REQUEST   = 80
- WRITE_PARAGRAPH_SUCCESS   = 81
- WRITE_PARAGRAPH_FAILURE   = 83
- SPEAKING_TOPIC_REQUEST    = 90
- SPEAKING_TOPIC_SUCCESS    = 91
- SPEAKING_TOPIC_FAILURE    = 93

// Notifications
- NOTIFICATION_PUSH = 140

────────────────────────────────────────────
3. Message Class
────────────────────────────────────────────
- MsgCode code
- std::vector<uint8_t> data (payload)
- serialize() / deserialize()
- Payload contains strings or JSON-like serialized data


────────────────────────────────────────────
5. Server Logic
────────────────────────────────────────────
1️⃣ Fetch Test/Exercise:
- Parse payload: <token>;<exercise_id or exam_id>
- Validate session token
- Load Exercise/Exam from DB
- Send *_SUCCESS message with content

2️⃣ Submit Answer:
- Receive payload: <token>;<target_type>;<target_id>;<user_answer>
- Validate session token
- Store submission in results table (score NULL if teacher grading required)
- If auto-graded, compute score and feedback
- Send *_SUCCESS or *_FAILURE
- If score/feedback ready, push NOTIFICATION_PUSH

3️⃣ Teacher Grading:
- Teacher updates results table with score/feedback
- Server pushes NOTIFICATION_PUSH to student(s)

4️⃣ View Done/Undone List:
- Client requests list of exercises/exams done or undone
- Server queries results table, showing:
  - Done: results with score/feedback filled
  - Pending: submitted but not graded
  - Not attempted: no record
- Return serialized list via Message

────────────────────────────────────────────
6. Client Logic
────────────────────────────────────────────
- Send REQUEST messages to fetch exercises/tests
- Display content to user
- Submit answers via REQUEST message
- Receive *_SUCCESS or *_FAILURE
- Receive asynchronous NOTIFICATION_PUSH with score/feedback
- Fetch list of done/undone exercises/tests
- Multiplexed socket read loop, maintain session and heartbeat

────────────────────────────────────────────
7. Heartbeat & Graceful Disconnect
────────────────────────────────────────────
- Heartbeat: client sends periodic HEARTBEAT; server resets timeout
- Graceful disconnect: client sends LOGOUT_REQUEST or disconnect; server cleans up session
- Server closes inactive connections after timeout

────────────────────────────────────────────
8. Detailed Logic Flow Diagrams
────────────────────────────────────────────
1️⃣ Do Test / Do Exercise
Client                          Server
  |                                |
  |-- REQUEST -------------------->|
  |  (token, exercise/exam ID)    |
  |                                |
  |   Validate token/session       |
  |   Load content from DB         |
  |                                |
  |<-- *_SUCCESS ------------------|
  |  (questions)                   |
  |                                |
  |  Display questions             |

2️⃣ Submit Answer
Client                               Server
  |                                     |
  |-- SUBMIT_ANSWER_REQUEST ----------->|
  |  <token>;<target_type>;<target_id>;<answer> |
  |                                     |
  |   Validate token/session           |
  |   Store in results table           |
  |   Compute score if auto-graded     |
  |                                     |
  |<-- *_SUCCESS / *_FAILURE ----------|
  |                                     |
  |  Display confirmation             |

3️⃣ Teacher Grading or Auto-Grading → Push Notify
Server                               Client
  |                                     |
  |-- NOTIFICATION_PUSH ---------------->|
  |  (score, feedback)                  |
  |                                     |
  |  Display to user                   |

4️⃣ View Done/Undone Exercises/Tests
Client                          Server
  |                                |
  |-- REQUEST_LIST ---------------->|
  |                                |
  |   Query results table           |
  |   Return status: done/pending   |
  |                                |
  |<-- RESPONSE_LIST ---------------|
  |  Display status to user         |

────────────────────────────────────────────
9. Additional Requirements
────────────────────────────────────────────
- Use existing Message class for all TCP messages
- Log all server messages
- Modular server handlers: exercise_handler, exercise_loader, result_handler, submission_handler
- Serialize/deserialize messages properly
- Maintain compatibility with login/logout, cookies, session, heartbeat, graceful disconnect
- Multiplexed socket read/write for both client and server
- Include comments explaining DB queries, session handling, message handling, and notification push
