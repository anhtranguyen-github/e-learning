# English Learning Platform - Socket Server

A multi-threaded C++ socket server for an English learning platform with PostgreSQL database integration.

## Features

- **User Authentication**: Secure login/logout system with PostgreSQL backend
- **Session Management**: Persistent session tracking with automatic expiration
- **Real-time Communication**: Socket-based client-server architecture
- **Database Integration**: Full PostgreSQL integration for data persistence
- **Thread Safety**: Mutex-protected operations for concurrent requests
- **Logging System**: Comprehensive logging for debugging and monitoring

## Architecture

```
┌─────────────────────────────────────────┐
│           Client Application            │
└─────────────┬───────────────────────────┘
              │ TCP Socket
┌─────────────▼───────────────────────────┐
│           Server (Port 8080)            │
├─────────────────────────────────────────┤
│  ClientHandler → SessionManager         │
│                   ↓                     │
│              UserManager                │
│                   ↓                     │
│           Database (libpq)              │
└─────────────┬───────────────────────────┘
              │
┌─────────────▼───────────────────────────┐
│        PostgreSQL Database              │
│  - users                                │
│  - server_sessions                      │
│  - lessons, exercises, exams            │
└─────────────────────────────────────────┘
```

## Tech Stack

- **Language**: C++17
- **Database**: PostgreSQL (libpq)
- **Networking**: POSIX Sockets
- **Concurrency**: POSIX Threads, Mutex
- **Build System**: GNU Make

## Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential g++ postgresql postgresql-contrib libpq-dev

# Verify installations
g++ --version
psql --version
```

## Quick Start

### 1. Clone and Setup Database

```bash
# Navigate to project
cd project

# Setup database (creates DB, schema, and test data)
cd database
./setup.sh
cd ..
```

### 2. Build Project

```bash
# Build server and client
make clean
make all
```

### 3. Run Server

```bash
./bin/server
# Server will start on port 8080
```

### 4. Run Client (in another terminal)

```bash
./bin/client
```

## Project Structure

```
project/
├── include/              # Header files
│   ├── common/          # Shared utilities
│   │   ├── logger.h
│   │   ├── protocol.h
│   │   └── utils.h
│   ├── server/          # Server components
│   │   ├── database.h
│   │   ├── user_manager.h
│   │   ├── session.h
│   │   ├── client_handler.h
│   │   └── server.h
│   └── client/          # Client components
│       ├── network.h
│       └── ui.h
├── src/                 # Implementation files
│   ├── common/
│   ├── server/
│   └── client/
├── database/            # Database setup
│   ├── init_db.sql
│   ├── seed_db.sql
│   ├── setup.sh
│   └── README.md
├── bin/                 # Compiled binaries
├── build/               # Object files
├── logs/                # Log files
└── Makefile
```

## Database Schema

### Users Table
```sql
CREATE TABLE users (
    user_id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    role VARCHAR(10) CHECK (role IN ('admin', 'teacher', 'student')),
    level VARCHAR(10) CHECK (level IN ('beginner', 'intermediate', 'advanced')),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### Server Sessions Table
```sql
CREATE TABLE server_sessions (
    session_id VARCHAR(255) PRIMARY KEY,
    user_id INTEGER NOT NULL,
    client_fd INTEGER,
    last_active TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    active BOOLEAN NOT NULL DEFAULT TRUE,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);
```

## Configuration

### Database Connection

Update connection string in `include/server/db_config.h`:

```cpp
static std::string getConnectionString() {
    return "host=localhost port=5432 dbname=english_learning user=postgres password=yourpass";
}
```

### Server Port

Default: 8080. Change in `src/server/server.cpp` main function:

```cpp
int port = 8080;  // Change this
```

## API/Protocol

### Message Format
```
[4 bytes: code][4 bytes: length][payload]
```

### Message Codes
- `LOGIN_REQUEST`: User login
- `LOGIN_SUCCESS`: Login successful
- `LOGIN_FAILURE`: Login failed
- `LOGOUT_REQUEST`: User logout
- `LOGOUT_SUCCESS`: Logout successful
- `HEARTBEAT`: Keep-alive ping
- `DISCONNECT_REQUEST`: Graceful disconnect

## Development

### Build Commands

```bash
# Clean build
make clean

# Build all (server + client)
make all

# Build server only
make bin/server

# Build client only
make bin/client

# Run server
make run-server

# Run client
make run-client
```

### Testing

```bash
# Test database connection
cd database
make
./db_test

# Check server logs
tail -f logs/server.log
```

### Debugging

```bash
# Compile with debug symbols
make CXXFLAGS="-std=c++17 -Wall -Wextra -I./include -pthread -g"

# Run with gdb
gdb ./bin/server
```

## Security Notes

⚠️ **WARNING**: Current implementation is for DEVELOPMENT ONLY

- Passwords stored in plain text (implement bcrypt/argon2)
- No SSL/TLS encryption
- Basic SQL injection protection (parameterized queries)
- No rate limiting
- Default credentials in code

**Production TODO:**
1. Hash passwords (bcrypt)
2. SSL/TLS for connections
3. Environment variables for secrets
4. Connection pooling
5. Rate limiting
6. Input validation
7. Security audit

## Performance

- Thread-safe operations with mutex locks
- Session caching (memory + DB)
- Connection reuse
- Prepared statements support

## Troubleshooting

### Database Connection Failed
```bash
# Check PostgreSQL is running
sudo service postgresql status
sudo service postgresql start

# Test connection
psql -U postgres -d english_learning
```

### Port Already in Use
```bash
# Find process using port
sudo lsof -i :8080

# Kill process
sudo kill -9 <PID>
```

### Build Errors
```bash
# Install missing dependencies
sudo apt-get install build-essential libpq-dev

# Clean and rebuild
make clean-all
make all
```

## Documentation

- [PostgreSQL Integration Guide](POSTGRESQL_INTEGRATION.md)
- [Database Setup](database/README.md)
- [Login/Logout Flow](login-logout-heartbeat-gracequit-instruct.md)

## Contributing

1. Create feature branch
2. Make changes
3. Test thoroughly
4. Submit pull request

## License

[Your License Here]

## Authors

[Your Name/Team]

## Version

1.0.0 - PostgreSQL Integration Complete
