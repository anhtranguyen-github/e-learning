# Overall
┌──────────────┐
│ Client Start │
└──────┬───────┘
       │
       ▼
┌────────────────────────┐
│ Connect to TCP Server  │
│ (Non-blocking socket)  │
└─────────┬──────────────┘
          │
          ▼
   ┌──────────────┐
   │ Login Phase  │
   └──────┬───────┘
          │
          ▼
   ┌──────────────────────────────┐
   │ Send LOGIN_REQUEST           │
   │ → payload: username;password │
   └─────────┬────────────────────┘
             │
             ▼
   ┌──────────────────────────────┐
   │ Server verifies credentials  │
   │   ├─ Valid → LOGIN_SUCCESS   │
   │   │     + session_token      │
   │   └─ Invalid → LOGIN_FAILURE │
   └─────────┬────────────────────┘
             │
             ▼
   ┌──────────────────────────┐
   │ Save session_token (cookie) │
   │ Start heartbeat timer     │
   └──────────┬────────────────┘
              │
              ▼
        ┌─────────────┐
        │ Active Loop │
        └────┬────────┘
             │
             ├──► Heartbeat Timer (every N sec)
             │     send HEARTBEAT message
             │     server resets timeout
             │
             ├──► Multiplexed I/O
             │     monitor for:
             │        • server messages (e.g. notification)
             │        • user commands (e.g. logout)
             │
             └──► User selects "Logout"
                    ↓
             ┌──────────────────────────────┐
             │ Send LOGOUT_REQUEST           │
             │ → payload: session_token      │
             └─────────┬────────────────────┘
                       │
                       ▼
             ┌──────────────────────────────┐
             │ Server validates token        │
             │ → closes session record       │
             │ → responds LOGOUT_SUCCESS     │
             └─────────┬────────────────────┘
                       │
                       ▼
             ┌────────────────────────┐
             │ Graceful Disconnect     │
             │ send DISCONNECT_REQUEST │
             │ wait DISCONNECT_ACK     │
             │ close socket cleanly    │
             └────────────────────────┘


# Server

       ┌──────────────────────────────┐
       │ For each client_fd (epoll)   │
       └─────────────┬────────────────┘
                     │
                     ▼
       ┌──────────────────────────────┐
       │ On LOGIN_SUCCESS:             │
       │   create session:             │
       │     token, username, expire   │
       │   start heartbeat timer       │
       └─────────────┬────────────────┘
                     │
                     ▼
       ┌──────────────────────────────┐
       │ On HEARTBEAT:                │
       │   update last_seen_time      │
       │   reset idle counter         │
       └─────────────┬────────────────┘
                     │
                     ▼
       ┌──────────────────────────────┐
       │ Timer check loop:            │
       │   if (now - last_seen_time > │
       │       heartbeat_timeout):    │
       │      send DISCONNECT_ACK     │
       │      close socket            │
       └──────────────────────────────┘


# Cookie/Session Logic

┌──────────────────────────────┐
│ Client stores session_token  │
│ (like cookie) after login    │
│ -> sent with every request   │
└──────────────┬───────────────┘
               │
               ▼
┌──────────────────────────────┐
│ Server validates token:      │
│   - Exists in session list?  │
│   - Not expired?             │
│ If valid → proceed           │
│ Else → send GENERAL_FAILURE  │
└──────────────────────────────┘
