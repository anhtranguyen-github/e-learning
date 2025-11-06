# PostgreSQL Integration Guide

## Overview

The project has been successfully integrated with PostgreSQL database. The old file-based `UserDatabase` has been replaced with a new `UserManager` class that uses PostgreSQL for persistent storage.

## Major Changes

### 1. Database Layer
- **New**: `Database` class (`include/server/database.h`, `src/server/database.cpp`)
  - PostgreSQL connection manager using libpq
  - Thread-safe operations with mutex protection
  - Parameterized query support for SQL injection prevention
  - Connection pooling ready

### 2. User Management
- **Renamed**: `UserDatabase` → `UserManager`
- **Files**:
  - `include/server/user_manager.h`
  - `src/server/user_manager.cpp`
- **Features**:
  - User authentication with PostgreSQL backend
  - User CRUD operations
  - Role and level management
  - Password updates

### 3. Session Management
- **Updated**: `SessionManager` class
- **Changes**:
  - Added database persistence for sessions
  - Sessions now stored in `server_sessions` table
  - Session data includes user_id for database linkage
  - Automatic session cleanup and expiration

### 4. Server Architecture
- **Updated**: `Server` class
- **Changes**:
  - Dependency injection pattern with shared_ptr
  - Database connection initialized on startup
  - Shared components: Database → UserManager → SessionManager → ClientHandler

## Class Hierarchy

```
Database (PostgreSQL Connection)
    ↓
UserManager (User Auth & Management)
    ↓
SessionManager (Session Persistence)
    ↓
ClientHandler (Request Processing)
    ↓
Server (Main Application)
```

## Database Schema

### users table
```sql
- user_id (SERIAL PRIMARY KEY)
- username (VARCHAR UNIQUE)
- password_hash (VARCHAR)
- full_name (VARCHAR)
- role (VARCHAR: admin|teacher|student)
- level (VARCHAR: beginner|intermediate|advanced)
- created_at (TIMESTAMP)
```

### server_sessions table
```sql
- session_id (VARCHAR PRIMARY KEY)
- user_id (INTEGER FK → users)
- client_fd (INTEGER)
- last_active (TIMESTAMP)
- active (BOOLEAN)
- created_at (TIMESTAMP)
- expires_at (TIMESTAMP)
```

## API Changes

### UserManager (formerly UserDatabase)

**Before:**
```cpp
UserDatabase db("data/users.txt");
bool valid = db.verifyCredentials(username, password);
```

**After:**
```cpp
auto userManager = std::make_shared<UserManager>(database);
bool valid = userManager->verifyCredentials(username, password);
int userId = userManager->getUserId(username);
string role = userManager->getUserRole(username);
```

### SessionManager

**Before:**
```cpp
SessionManager sm(30);
string sessionId = sm.createSession(username, clientFd);
```

**After:**
```cpp
auto sessionManager = std::make_shared<SessionManager>(database, 30);
string sessionId = sessionManager->createSession(username, userId, clientFd);
// Sessions automatically persisted to database
```

## Configuration

### Connection String Format
```
host=<hostname> port=<port> dbname=<database> user=<username> password=<password>
```

### Default Configuration
```cpp
// In db_config.h
"host=localhost port=5432 dbname=english_learning user=postgres password=yourpass"
```

### Environment Variables (Recommended for Production)
```bash
export DB_HOST=localhost
export DB_PORT=5432
export DB_NAME=english_learning
export DB_USER=postgres
export DB_PASSWORD=yourpass
```

## Build Instructions

### Prerequisites
```bash
sudo apt-get install libpq-dev postgresql-client
```

### Compile
```bash
make clean
make all
```

The Makefile now includes `-lpq` flag for PostgreSQL linking.

## Database Setup

### Quick Setup
```bash
cd database
./setup.sh
```

### Manual Setup
```bash
# Create database
sudo -u postgres psql -c "CREATE DATABASE english_learning;"

# Initialize schema
psql -U postgres -d english_learning -f database/init_db.sql

# Seed test data (optional)
psql -U postgres -d english_learning -f database/seed_db.sql
```

## Testing

### Database Connection Test
```bash
cd database
make
./db_test
```

### Server Test
```bash
make
./bin/server
```

## Migration Notes

### Removed Files
- Old implementation: `src/server/database_utils.cpp`, `include/server/database_utils.h`
- Old data storage: `data/users.txt` (no longer used)

### Updated Files
- `src/server/server.cpp`
- `src/server/client_handler.cpp`
- `src/server/session.cpp`
- `include/server/server.h`
- `include/server/client_handler.h`
- `include/server/session.h`
- `Makefile`

### New Files
- `include/server/database.h`
- `src/server/database.cpp`
- `include/server/user_manager.h`
- `src/server/user_manager.cpp`
- `include/server/db_config.h`
- `database/setup.sh`
- `database/README.md`
- `database/Makefile`

## Security Considerations

1. **Password Storage**: Currently storing plain text (FOR DEVELOPMENT ONLY)
   - TODO: Implement bcrypt or similar hashing

2. **SQL Injection**: Protected via parameterized queries
   - All user inputs use `PQexecParams`

3. **Connection Security**: 
   - TODO: Add SSL/TLS for production
   - TODO: Use connection pooling

4. **Session Security**:
   - Sessions stored with expiration
   - Automatic cleanup of expired sessions

## Performance

- Thread-safe database operations with mutex
- Prepared statement support
- Connection reuse
- Session caching in memory + DB persistence

## Future Enhancements

1. Connection pooling for concurrent requests
2. Redis cache for session data
3. Database migrations system
4. Monitoring and logging
5. Backup and recovery procedures
6. Replication for high availability

## Troubleshooting

### Connection Failed
```
Error: Connection to database failed
```
**Solution**: Check PostgreSQL service is running:
```bash
sudo service postgresql status
sudo service postgresql start
```

### Authentication Failed
```
Error: password authentication failed
```
**Solution**: Update password in database and code:
```sql
ALTER USER postgres WITH PASSWORD 'newpassword';
```

### Database Not Found
```
Error: database "english_learning" does not exist
```
**Solution**: Run database setup:
```bash
cd database
./setup.sh
```

## Support

For issues or questions:
1. Check database connection string
2. Verify PostgreSQL is running
3. Check logs in `logs/server.log`
4. Review database/README.md
