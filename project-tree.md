📦 TCP Learning App Project
├── include/                           # Header files (interfaces, declarations)
│   ├── client/                        # Client-side headers
│   │   ├── network.h                  # Socket connection, multiplexing, send/recv logic
│   │   └── ui.h                       # Command-line UI for user interaction
│   ├── common/                        # Shared components between client and server
│   │   ├── logger.h                   # Lightweight logging utility (info, warn, error)
│   │   ├── protocol.h                 # Message code definitions, serialization, framing
│   │   └── utils.h                    # Helper functions for string parsing, token validation, etc.
│   └── server/                        # Server-side headers
│       ├── chat_handler.h             # Handle chat message send/receive (130–135)
│       ├── client_handler.h           # Main client connection loop (select/poll logic)
│       ├── database_utils.h           # File-based or DB-backed user/session storage
│       ├── exercise_handlers.h        # Handle test & practice exercise requests (40–93)
│       ├── exercise_loader.h          # Load exercise content from .txt or DB
│       ├── lesson_handler.h           # Handle lesson list & study requests (110–123)
│       ├── lesson_loader.h            # Load lesson metadata/content
│       ├── server.h                   # Server initialization, socket setup, multiplex loop
│       └── session.h                  # Manage active sessions and authentication
└── src/                               # Implementation source files
    ├── client/
    │   ├── client.cpp                 # Client entry point; main loop & command routing
    │   ├── network.cpp                # Implements network operations declared in network.h
    │   └── ui.cpp                     # Implements UI logic for menus and input
    ├── common/
    │   ├── logger.cpp                 # Logging system implementation
    │   ├── protocol.cpp               # Message serialization/deserialization logic
    │   └── utils.cpp                  # Shared utility function implementations
    └── server/
        ├── chat_handler.cpp           # Implements real-time chat & message broadcast
        ├── client_handler.cpp         # Manages multiple clients with select()/poll()
        ├── database_utils.cpp         # Read/write user data, exercises, sessions
        ├── exercise_handlers.cpp      # Evaluate tests, process exercise submissions
        ├── exercise_loader.cpp        # Load exercises from files or database
        ├── lesson_handler.cpp         # Respond to lesson list & study lesson requests
        ├── lesson_loader.cpp          # Parse and provide lesson content (video, audio, text, etc.)
        └── server.cpp                 # Server main entry, event loop, and push notifications
