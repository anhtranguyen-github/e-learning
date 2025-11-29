# Chat Protocol Design

## Overview
The Chat protocol enables private messaging between users and retrieval of chat history.

## Messages

| Message Name | Code | Description | Payload Structure |
| :--- | :--- | :--- | :--- |
| `SEND_CHAT_PRIVATE_REQUEST` | 300 | Send a private message. | `sessionToken;recipient;messageType;content` |
| `CHAT_PRIVATE_RECEIVE` | 301 | Receive a private message (push). | `sender;messageType;content;timestamp` |
| `CHAT_MESSAGE_SUCCESS` | 302 | Message sent successfully. | `success_message` |
| `CHAT_MESSAGE_FAILURE` | 303 | Failed to send message. | `error_message` |
| `CHAT_HISTORY_REQUEST` | 304 | Request chat history with a user. | `sessionToken;otherUser` |
| `CHAT_HISTORY_SUCCESS` | 305 | Returns chat history. | List of messages |
| `CHAT_HISTORY_FAILURE` | 306 | Failed to retrieve history. | `error_message` |

## Payload Definitions

### PrivateMessageRequest
### PrivateMessageRequest
- **Fields**: `sessionToken`, `recipient`, `messageType`, `content`
- **Serialization**: `sessionToken;recipient;messageType;content`
- **Note**: `messageType` can be "TEXT" or "AUDIO". If "AUDIO", `content` is a Base64 encoded string of the audio file.

### ChatHistoryRequest
- **Fields**: `sessionToken`, `otherUser`
- **Serialization**: `sessionToken;otherUser`

## Example Flow

### Sending a Message (Text)
1.  **Client A** sends `SEND_CHAT_PRIVATE_REQUEST`:
    ```
    Code: 300
    Payload: "tokenA;UserB;TEXT;Hello there!"
    ```
2.  **Server** forwards to **Client B** as `CHAT_PRIVATE_RECEIVE`:
    ```
    Code: 301
    Payload: "UserA;TEXT;Hello there!;2023-10-27 10:00:00"
    ```

### Sending a Message (Audio)
1.  **Client A** records audio, saves to temp file, reads bytes, and Base64 encodes them.
2.  **Client A** sends `SEND_CHAT_PRIVATE_REQUEST`:
    ```
    Code: 300
    Payload: "tokenA;UserB;AUDIO;<Base64EncodedAudioData>"
    ```
3.  **Server** receives, decodes (optional, or stores as blob/file), and forwards to **Client B**.
4.  **Client B** receives `CHAT_PRIVATE_RECEIVE`:
    ```
    Code: 301
    Payload: "UserA;AUDIO;<Base64EncodedAudioData>;2023-10-27 10:05:00"
    ```
5.  **Client B** detects `AUDIO` type:
    - Decodes Base64 data to binary.
    - Checks **User Settings** for "Audio Download Folder".
    - Saves file as `audio_timestamp_sender.wav` in that folder.
    - Displays an audio player in the chat UI pointing to that local file.

### Retrieving History
1.  **Client** sends `CHAT_HISTORY_REQUEST`:
    ```
    Code: 304
    Payload: "tokenA;UserB"
    ```
2.  **Server** responds with `CHAT_HISTORY_SUCCESS`:
    ```
    Code: 305
    Payload: "UserB|TEXT|Hi|time^UserA|AUDIO|<Base64Data>|time" 
    ```

