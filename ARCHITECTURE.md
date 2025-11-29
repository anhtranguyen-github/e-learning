# Client-Server Communication Architecture

This section documents the logic, flow, and coordination between the GUI Client, Network Manager, Network Client, and Protocol definitions.

## 1. Components Overview

*   **`src/client/gui/NetworkManager.cpp` (GUI Layer)**:
    *   Acts as a bridge between the Qt/QML UI and the C++ Network Layer.
    *   Manages the `client::NetworkClient` instance.
    *   Uses a `QTimer` to poll for incoming messages every 100ms.
    *   Converts QML function calls into network requests.
    *   Converts incoming network messages into Qt Signals for the UI.

*   **`src/client/network.cpp` (Network Layer)**:
    *   Handles raw TCP socket operations (connect, send, recv).
    *   Manages connection state (`connected`, `loggedIn`) and `sessionToken`.
    *   Implements specific request methods (e.g., `requestLessonList`) that construct payloads and send messages.
    *   Provides `pollMessages()` to read and reassemble fragmented TCP packets into complete messages.

*   **`include/common/protocol.h` (Protocol Layer)**:
    *   Defines `MsgCode` enum for all message types (e.g., `LOGIN_REQUEST`, `LESSON_LIST_SUCCESS`).
    *   Defines the binary `Message` structure: `[Length (4B)][Code (2B)][Payload (NB)]`.
    *   Handles binary serialization/deserialization of the message frame.

*   **`include/common/payloads.h` (Payload Layer)**:
    *   Defines Data Transfer Objects (DTOs) for specific requests/responses (e.g., `LessonListRequest`, `LessonDTO`).
    *   Handles string-based serialization of the payload data (typically delimiter-separated, e.g., `;` or `|`).

## 2. Coordination Flow

### A. Connection & Login
1.  **UI** calls `NetworkManager::connectToServer`.
2.  **NetworkManager** calls `m_client->connect()`, which establishes the TCP connection.
3.  **UI** calls `NetworkManager::login(user, pass)`.
4.  **NetworkManager** stops the polling timer (synchronous operation).
5.  **NetworkManager** calls `m_client->login()`.
    *   Constructs `Payloads::LoginRequest`.
    *   Wraps in `protocol::Message` (`LOGIN_REQUEST`).
    *   Sends and **blocks** waiting for `LOGIN_SUCCESS` or `LOGIN_FAILURE`.
    *   On success, extracts `session_id` and stores it in `m_client`.
6.  **NetworkManager** emits `loginSuccess` and restarts the polling timer.

### B. Sending Requests (e.g., Get Lesson List)
1.  **UI** calls `NetworkManager::requestLessonList(topic, level)`.
2.  **NetworkManager** delegates to `m_client->requestLessonList()`.
3.  **NetworkClient**:
    *   Checks `loggedIn` state.
    *   Creates `Payloads::LessonListRequest` with the stored `sessionToken`.
    *   Serializes payload: `"token;topic;level"`.
    *   Creates `protocol::Message` with code `LESSON_LIST_REQUEST`.
    *   Serializes message: `[Len][Code][Payload]`.
    *   Sends bytes over socket.
    *   Returns `true` (request sent).

### C. Receiving Responses (Polling Loop)
1.  **NetworkManager**'s `QTimer` fires `checkMessages()`.
2.  **NetworkManager** calls `m_client->pollMessages()`.
3.  **NetworkClient**:
    *   Reads available bytes from socket (non-blocking).
    *   Appends to internal buffer.
    *   Uses `protocol::Message::getFullLength()` to check for complete messages.
    *   Deserializes and returns a vector of `protocol::Message` objects.
4.  **NetworkManager** iterates through messages:
    *   Switches on `msg.code`.
    *   **Example**: If `LESSON_LIST_SUCCESS`:
        *   Extracts payload string (e.g., `"id|title...;id2|title2..."`).
        *   Emits signal `lessonListReceived(payload)`.
    *   **Example**: If `LESSON_LIST_FAILURE`:
        *   Emits `errorOccurred(errorMessage)`.
5.  **UI** receives the signal and updates the view.

## 3. Key Data Structures

*   **Protocol Message**:
    ```cpp
    struct Message {
        MsgCode code;
        std::vector<uint8_t> data; // Payload
        // Serializes to: [TotalLen (4B)][MsgCode (2B)][Data...]
    };
    ```

*   **Payload Serialization**:
    *   Most payloads use `;` as a field separator (e.g., `LoginRequest`).
    *   Lists often use `|` to separate items (e.g., `LessonDTO` list).
    *   Nested lists might use `^` or `,`.
    *   *Note*: This string-based serialization is defined in `payloads.h`.

## 4. Serialization Hierarchy (Coordination)

The system uses a layered approach to serialization, moving from high-level business objects to low-level network bytes.

### Layer 1: Model Classes (Server-Side)
*   **Location**: `src/server/model/` (e.g., `Lesson`, `LessonList`)
*   **Role**: Represents the core business entities in memory.
*   **Serialization Logic**:
    *   `toDTO()`: Converts the internal model into a `Payloads` DTO. This is the **preferred** modern way.
    *   `serializeForNetwork()`: Legacy method that manually builds a string. **Deprecated** in favor of DTOs.
    *   *Example*: `Lesson::toDTO()` creates a `Payloads::LessonDTO` and populates it with `id`, `title`, etc.

### Layer 2: Payloads (Shared)
*   **Location**: `include/common/payloads.h`
*   **Role**: Defines the standard data format for transmission.
*   **Serialization Logic**:
    *   `serialize()`: Converts the DTO struct into a single string (e.g., `"1|Title|Topic|Level"`).
    *   `deserialize()`: Parses that string back into the DTO struct.
    *   *Example*: `LessonDTO::serialize()` joins all fields with `|`.

### Layer 3: Protocol Message (Shared)
*   **Location**: `include/common/protocol.h`
*   **Role**: Wraps the payload string in a binary envelope for TCP.
*   **Serialization Logic**:
    *   `serialize()`: Creates a byte vector `[Length][Code][PayloadString]`.
    *   `deserialize()`: Extracts the `PayloadString` from the byte vector.

### End-to-End Flow Example (Sending a Lesson)

1.  **Database**: Server reads data and creates a `server::Lesson` object.
2.  **Model -> DTO**: Server calls `lesson.toDTO()` -> returns `Payloads::LessonDTO`.
3.  **DTO -> String**: Server calls `dto.serialize()` -> returns `std::string` payload.
4.  **String -> Message**: Server creates `protocol::Message(LESSON_SUCCESS, payload)`.
5.  **Message -> Bytes**: Server calls `msg.serialize()` -> returns `std::vector<uint8_t>`.
6.  **Network**: Bytes are sent over TCP.
7.  **Client**: Receives bytes, reconstructs `protocol::Message`.
8.  **Message -> String**: Client extracts `msg.toString()`.
9.  **String -> DTO**: Client creates `Payloads::LessonDTO` and calls `dto.deserialize(str)`.
10. **UI**: Client uses DTO fields to display data.
