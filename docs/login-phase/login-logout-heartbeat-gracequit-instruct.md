# System Design Summary

**Scope:** Login / Logout, Cookies & Sessions, Heartbeat, Graceful Disconnect, and Multiplexing Architecture
**Language:** C++ (Client–Server, TCP Socket)
**Directory Structure:** follows `/include` and `/src` separation for client, server, and common modules.

---

## 1. Multiplexing Model

* **Goal:** Handle multiple clients simultaneously using non-blocking sockets.
* **Implementation:** Both client and server use `select()` for I/O multiplexing.
* **Server:**

  * Maintains a list of active sockets.
  * When a socket is readable → receive message → dispatch by `MsgCode`.
  * When writable → send queued messages.
* **Client:**

  * Reads both user input and server push (e.g., notifications) concurrently.
* **Advantage:** Single-threaded, scalable, event-driven network loop.

---

## 2. Login & Session Management

### 2.1 Login Flow

1. **Client → Server:**
   `MsgCode = LOGIN_REQUEST (10)`
   `Payload = username;password`

2. **Server:**

   * Verifies credentials in user database (`users.txt`).
   * If valid → generate `session_id` (UUID).
   * Store in `session_table[session_id]` with username and timestamp.
   * Respond:

     ```
     MsgCode = LOGIN_SUCCESS (11)
     Payload = "Set-Cookie: session_id=<uuid>"
     ```
   * Else respond with `LOGIN_FAILURE (13)`.

3. **Client:**

   * Extracts `session_id` from response.
   * Stores in `stored_cookie`.
   * Adds cookie in every future request:

     ```
     Header: Cookie: session_id=<uuid>
     ```

---

## 3. Session Validation

* For every incoming request, the server checks:

  * If cookie exists in `session_table`.
  * If `active == true` and not expired.
* Otherwise, server replies with `GENERAL_FAILURE (253)` and closes connection.

---

## 4. Logout Flow

1. **Client → Server:**
   `MsgCode = LOGOUT_REQUEST (20)`
   `Payload = "Cookie: session_id=<uuid>"`
2. **Server:**

   * Locate session and mark `active = false`.
   * Remove entry from `session_table`.
   * Respond `LOGOUT_SUCCESS (21)` and close connection gracefully.

---

## 5. Heartbeat Mechanism

* **Purpose:** Keep session alive and detect dead sockets.
* **Client → Server periodically:**
  `MsgCode = HEARTBEAT (30)`
  `Payload = "Cookie: session_id=<uuid>"`
* **Server:**

  * Updates `last_active` timestamp in session table.
  * If no heartbeat > N seconds → mark inactive, close socket.
* Prevents zombie connections in multiplexing loop.

---

## 6. Graceful Disconnect

* Triggered when:

  * Client explicitly logs out.
  * Network timeout.
  * Server shutdown.

* **Server Steps:**

  1. Send `GENERAL_FAILURE (253)` or `LOGOUT_SUCCESS (21)` if applicable.
  2. Flush remaining outgoing data.
  3. Close socket and remove from active list.
  4. Log event: `[INFO] Disconnected client <fd> (session_id=<uuid>)`.

* **Client Steps:**

  1. On server disconnect → release resources.
  2. Log `[WARN] Connection closed by server.`

---

## 7. Logging

* Centralized logger (`common/logger.h`):

  * `[INFO] Connection established`
  * `[DEBUG] Received MsgCode=10`
  * `[WARN] Session expired`
  * `[ERROR] Invalid credentials`

All logs stored in `logs/server.log` and `logs/client.log`.

---

## 8. Summary Flow Diagram

```
Client                               Server
  │                                    │
  │--- LOGIN_REQUEST (10) ------------>│
  │                                    │ validate user
  │                                    │ create session_id
  │<-- LOGIN_SUCCESS (11) -------------│
  │ save cookie                        │
  │                                    │
  │--- HEARTBEAT (30) ---------------->│ update last_active
  │                                    │
  │--- LOGOUT_REQUEST (20) ----------->│ deactivate session
  │<-- LOGOUT_SUCCESS (21) ------------│ close socket
  │                                    │
```

---

## 9. Directory Reference

```
├── include
│   ├── client/         # UI, network
│   ├── common/         # logger, protocol, utils
│   └── server/         # handlers, session, db, multiplexing core
└── src
    ├── client/         # client.cpp, network.cpp
    ├── common/         # logger.cpp, protocol.cpp
    └── server/         # server.cpp, client_handler.cpp
```

**Core logic files:**

* `protocol.h / protocol.cpp` → MsgCode definitions, framing, serialization.
* `session.h / session.cpp` → session map, cookie validation.
* `server.cpp` → main multiplexing loop with select().
* `logger.cpp` → unified logging system.

---

✅ **Summary:**
This design defines a clean and extensible foundation for a C++ TCP-based e-learning system.
It supports secure session management (cookie-based), multiplexed networking, automatic heartbeat validation, and graceful shutdown handling for all active users.
 
 """
 enum class MsgCode : uint16_t {
    // Session and login
    LOGIN_REQUEST = 100,
    LOGIN_SUCCESS = 101,
    LOGIN_FAILURE = 103,

    LOGOUT_REQUEST = 200,
    LOGOUT_SUCCESS = 201,

    // Heartbeat
    HEARTBEAT = 900,

    // Graceful disconnect
    DISCONNECT_REQUEST = 901,
    DISCONNECT_ACK = 902,

    // General errors
    GENERAL_FAILURE = 9993,
    UNKNOWN_COMMAND_FAILURE = 9994
};

struct Message {
    MsgCode code;
    std::vector<uint8_t> data;

    Message() = default;
    Message(MsgCode c, const std::vector<uint8_t>& d) : code(c), data(d) {}
    Message(MsgCode c, const std::string& s)
        : code(c), data(s.begin(), s.end()) {}

    std::string toString() const {
        return std::string(data.begin(), data.end());
    }

    // ── Serialize: [2 bytes code][payload bytes...]
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> packet;
        packet.reserve(2 + data.size());
        uint16_t code_net = htons(static_cast<uint16_t>(code));
        uint8_t* p = reinterpret_cast<uint8_t*>(&code_net);
        packet.insert(packet.end(), p, p + 2);
        packet.insert(packet.end(), data.begin(), data.end());
        return packet;
    }

    // ── Deserialize
    static Message deserialize(const std::vector<uint8_t>& buffer) {
        if (buffer.size() < 2) throw std::runtime_error("Invalid packet");
        uint16_t code_net;
        std::memcpy(&code_net, buffer.data(), 2);
        MsgCode c = static_cast<MsgCode>(ntohs(code_net));
        std::vector<uint8_t> payload(buffer.begin() + 2, buffer.end());
        return Message(c, payload);
    }
};
"""