# Chat Protocol Design

## Overview
The Chat protocol enables private messaging between users and retrieval of chat history.

## Messages

| Message Name | Code | Description | Payload Structure |
| :--- | :--- | :--- | :--- |
| `SEND_CHAT_PRIVATE_REQUEST` | 300 | Send a private message. | `sessionToken;recipient;message` |
| `CHAT_PRIVATE_RECEIVE` | 301 | Receive a private message (push). | `sender;message;timestamp` (implied) |
| `CHAT_MESSAGE_SUCCESS` | 302 | Message sent successfully. | `success_message` |
| `CHAT_MESSAGE_FAILURE` | 303 | Failed to send message. | `error_message` |
| `CHAT_HISTORY_REQUEST` | 304 | Request chat history with a user. | `sessionToken;otherUser` |
| `CHAT_HISTORY_SUCCESS` | 305 | Returns chat history. | List of messages |
| `CHAT_HISTORY_FAILURE` | 306 | Failed to retrieve history. | `error_message` |

## Payload Definitions

### PrivateMessageRequest
- **Fields**: `sessionToken`, `recipient`, `message`
- **Serialization**: `sessionToken;recipient;message`

### ChatHistoryRequest
- **Fields**: `sessionToken`, `otherUser`
- **Serialization**: `sessionToken;otherUser`

## Example Flow

### Sending a Message
1.  **Client A** sends `SEND_CHAT_PRIVATE_REQUEST`:
    ```
    Code: 300
    Payload: "tokenA;UserB;Hello there!"
    ```
2.  **Server** forwards to **Client B** as `CHAT_PRIVATE_RECEIVE` (if online) or stores it.
    ```
    Code: 301
    Payload: "UserA;Hello there!"
    ```
3.  **Server** responds to **Client A** with `CHAT_MESSAGE_SUCCESS`:
    ```
    Code: 302
    Payload: "Message sent"
    ```

### Retrieving History
1.  **Client** sends `CHAT_HISTORY_REQUEST`:
    ```
    Code: 304
    Payload: "tokenA;UserB"
    ```
2.  **Server** responds with `CHAT_HISTORY_SUCCESS`:
    ```
    Code: 305
    Payload: "UserB:Hi|UserA:Hello|UserB:How are you?" 
    ```
    *(Note: History format depends on implementation, typically a list of serialized message objects).*
