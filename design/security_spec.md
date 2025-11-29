# Security Specification

## Overview
Security in the Socker Learning Platform is built on three pillars: Authentication, Session Management, and Role-Based Access Control (RBAC).

## 1. Authentication

### Mechanism
-   **Credentials**: Username and Password.
-   **Verification**: Server verifies credentials against the `users` table.
-   **Hashing**: *Note: Currently, passwords are stored in plain text for development simplicity. In production, they MUST be hashed using a strong algorithm like Argon2 or bcrypt.*

### Flow
1.  Client sends `LOGIN_REQUEST` (username, password).
2.  Server checks DB.
3.  If valid, Server generates a **Session Token**.
4.  Server returns `LOGIN_SUCCESS` with the token and user role.

## 2. Session Management

### Session Token
-   **Format**: UUID string.
-   **Storage**:
    -   **Server**: In-memory map `std::map<std::string, Session>`.
    -   **Client**: Stored in `NetworkClient` memory.
-   **Expiration**: Sessions expire after a fixed period of inactivity (e.g., 30 minutes).

### Validation
-   Every request (except Login/Register) must include the `session_token` in the payload.
-   `RBACMiddleware` validates the token before processing the request.

## 3. Role-Based Access Control (RBAC)

### Roles
1.  **Student**: Can view lessons, take exercises/exams, view own results, chat.
2.  **Teacher**: Can view lessons, grade submissions, view all results, chat.
3.  **Admin**: Full system access.

### Middleware Implementation
The `RBACMiddleware` intercepts every request:
1.  **Extract Token**: Parses `session_token` from the message payload.
2.  **Validate Session**: Checks if the token exists and is active in `SessionManager`.
3.  **Check Permissions**: Compares the user's role against the required role for the requested `MsgCode`.

### Permission Matrix (Example)

| MsgCode | Allowed Roles |
| :--- | :--- |
| `LOGIN_REQUEST` | All (Public) |
| `LESSON_LIST_REQUEST` | Student, Teacher |
| `EXERCISE_LIST_REQUEST` | Student |
| `GRADE_SUBMISSION_REQUEST` | Teacher |
| `PENDING_SUBMISSIONS_REQUEST`| Teacher |

## Security Best Practices (Planned)
-   **TLS/SSL**: Encrypt TCP traffic to prevent eavesdropping.
-   **Input Validation**: Sanitize all inputs to prevent SQL Injection (already handled by `libpq` parameterized queries).
-   **Rate Limiting**: Prevent brute-force attacks on login.
