# System Protocol Design

## Overview
The System protocol handles connection maintenance, notifications, and general error reporting.

## Messages

| Message Name | Code | Description | Payload Structure |
| :--- | :--- | :--- | :--- |
| `HEARTBEAT` | 900 | Keep-alive signal. | Empty or Timestamp |
| `DISCONNECT_REQUEST` | 901 | Client requests graceful disconnect. | `sessionToken` |
| `DISCONNECT_ACK` | 902 | Server acknowledges disconnect. | Empty |
| `NOTIFICATION_PUSH` | 290 | Server pushes a notification. | `message` |
| `GENERAL_FAILURE` | 990 | Generic error. | `error_message` |
| `UNKNOWN_COMMAND_FAILURE` | 991 | Unknown message code received. | `bad_code` |

## Payload Definitions

### GenericResponse
- **Fields**: `success`, `message`
- **Serialization**: `1;msg` or `0;msg`

## Example Flow

### Heartbeat
1.  **Client** sends `HEARTBEAT`:
    ```
    Code: 900
    Payload: ""
    ```
2.  **Server** (optionally) responds with `HEARTBEAT` to confirm aliveness.

### Disconnect
1.  **Client** sends `DISCONNECT_REQUEST`:
    ```
    Code: 901
    Payload: "token123"
    ```
2.  **Server** cleans up session and responds with `DISCONNECT_ACK`:
    ```
    Code: 902
    Payload: ""
    ```
