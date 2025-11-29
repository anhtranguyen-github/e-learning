# Authentication Protocol Design

## Overview
The Authentication protocol handles user session management, including login and logout operations.

## Messages

| Message Name | Code | Description | Payload Structure |
| :--- | :--- | :--- | :--- |
| `LOGIN_REQUEST` | 100 | Client requests to log in. | `username;password` |
| `LOGIN_SUCCESS` | 101 | Server confirms successful login. | `session_token` (implied, usually part of success response or handled via state) |
| `LOGIN_FAILURE` | 102 | Server rejects login. | `error_message` |
| `LOGOUT_REQUEST` | 110 | Client requests to log out. | `session_token` |
| `LOGOUT_SUCCESS` | 111 | Server confirms logout. | `success_flag;message` |

## Payload Definitions

### LoginRequest
- **Fields**:
  - `username` (string): The user's identifier.
  - `password` (string): The user's password.
- **Serialization**: `username;password`

### GenericResponse (Used for Logout/Success/Failure)
- **Fields**:
  - `success` (bool): 1 for true, 0 for false.
  - `message` (string): Description or error message.
- **Serialization**: `1;Success message` or `0;Error message`

## Example Flow

### Successful Login
1.  **Client** sends `LOGIN_REQUEST`:
    ```
    Code: 100
    Payload: "user1;secret123"
    ```
2.  **Server** validates credentials.
3.  **Server** responds with `LOGIN_SUCCESS`:
    ```
    Code: 101
    Payload: "session_token_abc123" 
    ```
    *(Note: The exact payload for LOGIN_SUCCESS isn't explicitly defined as a struct in `payloads.h`, but typically returns the session ID or a success message).*

### Failed Login
1.  **Client** sends `LOGIN_REQUEST`:
    ```
    Code: 100
    Payload: "user1;wrongpass"
    ```
2.  **Server** responds with `LOGIN_FAILURE`:
    ```
    Code: 102
    Payload: "Invalid credentials"
    ```
