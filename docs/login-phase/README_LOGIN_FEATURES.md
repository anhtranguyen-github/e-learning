# Login, Logout, Heartbeat, and Graceful Disconnect Features

## Overview

This document describes the implementation of the authentication and session management features for the TCP Learning App, including:

- **Login/Logout** - User authentication with session management
- **Cookie-based Sessions** - Session tokens stored and validated
- **Heartbeat Mechanism** - Keep-alive messages to maintain sessions
- **Graceful Disconnect** - Clean connection termination

## Architecture

### Protocol Layer (`protocol.h`)

Message codes for authentication:
- `LOGIN_REQUEST (100)` - Client sends credentials
- `LOGIN_SUCCESS (101)` - Server confirms login with session token
- `LOGIN_FAILURE (103)` - Server rejects login
- `LOGOUT_REQUEST (200)` - Client requests logout
- `LOGOUT_SUCCESS (201)` - Server confirms logout
- `HEARTBEAT (900)` - Client sends keep-alive
- `DISCONNECT_REQUEST (901)` - Client requests disconnect
- `DISCONNECT_ACK (902)` - Server acknowledges disconnect

### Session Management (`session.h/cpp`)

**SessionManager** handles:
- Creating sessions with UUID tokens
- Validating session tokens
- Tracking last active time
- Expiring inactive sessions (default: 30 seconds timeout)
- Mapping file descriptors to sessions

### User Database (`database_utils.h/cpp`)

**UserDatabase** provides:
- File-based user storage (`data/users.txt`)
- Credential verification
- Default users: `admin/admin123`, `user1/password1`, `test/test`

### Client Network (`network.h/cpp`)

**NetworkClient** implements:
- Non-blocking TCP connection
- Login with username/password
- Logout with session token
- Automatic heartbeat sending (every 10 seconds)
- Graceful disconnect handling

### Server (`server.h/cpp`)

**Server** features:
- Non-blocking socket with `select()` multiplexing
- Handles multiple concurrent clients
- Periodic session expiration checks (every 5 seconds)
- Graceful client disconnect handling

## Flow Diagrams

### Login Flow

```
Client                               Server
  │                                    │
  │--- LOGIN_REQUEST (username;pwd) ->│
  │                                    │ Verify credentials
  │                                    │ Create session
  │                                    │ Generate token
  │<-- LOGIN_SUCCESS (Set-Cookie) ----│
  │ Store session_id                   │
  │                                    │
```

### Heartbeat Flow

```
Client                               Server
  │                                    │
  │--- HEARTBEAT (Cookie: token) ---->│
  │                                    │ Validate token
  │                                    │ Update last_active
  │                                    │
  (Repeat every 10 seconds)
```

### Logout Flow

```
Client                               Server
  │                                    │
  │--- LOGOUT_REQUEST (Cookie) ------>│
  │                                    │ Validate token
  │                                    │ Remove session
  │<-- LOGOUT_SUCCESS -----------------│
  │ Clear session_id                   │
  │                                    │
```

### Graceful Disconnect

```
Client                               Server
  │                                    │
  │--- DISCONNECT_REQUEST ----------->│
  │<-- DISCONNECT_ACK -----------------│
  │ Close socket                       │ Close socket
  │                                    │ Remove session
```

## Building and Running

### Compile

```bash
make
```

This creates:
- `bin/server` - Server executable
- `bin/client` - Client executable

### Run Server

```bash
make run-server
# or
./bin/server [port]
```

Default port: 8080

### Run Client

```bash
make run-client
# or
./bin/client [host] [port]
```

Default: 127.0.0.1:8080

## Usage

### Client Menu

1. **Connect to Server** - Establish TCP connection
2. **Login** - Authenticate with username/password
3. **View Status** - Display session information
4. **Logout** - End session and clear token
5. **Disconnect** - Close connection gracefully

### Default Users

- Username: `admin`, Password: `admin123`
- Username: `user1`, Password: `password1`
- Username: `test`, Password: `test`

## Features Implemented

### ✅ Login System
- Username/password authentication
- Session token generation (UUID format)
- Cookie-based session storage
- Login success/failure responses

### ✅ Session Management
- Thread-safe session storage
- Session validation on every request
- File descriptor to session mapping
- Automatic session cleanup

### ✅ Heartbeat Mechanism
- Client sends heartbeat every 10 seconds
- Server updates last active timestamp
- Sessions expire after 30 seconds of inactivity
- Automatic expired session cleanup every 5 seconds

### ✅ Graceful Disconnect
- Proper disconnect request/acknowledgment
- Session cleanup on disconnect
- Socket closure with proper shutdown
- Logout before disconnect

### ✅ Multiplexing
- Non-blocking sockets on both sides
- `select()` for I/O multiplexing
- Multiple concurrent client support
- Event-driven architecture

### ✅ Logging
- Separate logs for server and client
- Log levels: DEBUG, INFO, WARN, ERROR
- Timestamped log entries
- Files: `logs/server.log`, `logs/client.log`

## File Structure

```
include/
├── client/
│   ├── network.h        # Client network operations
│   └── ui.h             # Client user interface
├── common/
│   ├── logger.h         # Logging system
│   ├── protocol.h       # Message protocol
│   └── utils.h          # Utility functions
└── server/
    ├── client_handler.h # Message processing
    ├── database_utils.h # User database
    ├── server.h         # Main server
    └── session.h        # Session management

src/
├── client/
│   ├── client.cpp       # Client main
│   ├── network.cpp      # Network implementation
│   └── ui.cpp           # UI implementation
├── common/
│   ├── logger.cpp       # Logger implementation
│   └── utils.cpp        # Utilities implementation
└── server/
    ├── client_handler.cpp
    ├── database_utils.cpp
    ├── server.cpp       # Server main + implementation
    └── session.cpp      # Session implementation
```

## Testing

1. Start the server:
   ```bash
   ./bin/server
   ```

2. Start one or more clients:
   ```bash
   ./bin/client
   ```

3. Test scenarios:
   - Login with valid credentials
   - Login with invalid credentials
   - Observe heartbeat messages in logs
   - Wait 30+ seconds without heartbeat (session expires)
   - Logout and verify session cleanup
   - Disconnect and reconnect

## Logs

Check the logs for detailed operation:

```bash
# Server logs
tail -f logs/server.log

# Client logs
tail -f logs/client.log
```

## Security Notes

⚠️ **This is a demonstration implementation**:
- Passwords are stored in plain text
- No encryption on network communication
- Simple token generation
- For production, use:
  - Hashed passwords (bcrypt, argon2)
  - TLS/SSL encryption
  - Cryptographically secure tokens
  - Rate limiting
  - Additional validation

## Troubleshooting

**Connection refused:**
- Ensure server is running
- Check port is not in use: `netstat -an | grep 8080`

**Session expires too quickly:**
- Adjust `heartbeatTimeout` in `Server` constructor
- Adjust `heartbeatInterval` in `NetworkClient` constructor

**Build errors:**
- Ensure g++ with C++17 support
- Check all header files are in place
- Run `make clean` and rebuild
