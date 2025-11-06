# PostgreSQL Integration - Quick Reference Card

## 🚀 Quick Start (5 Minutes)

```bash
# 1. Setup database
cd database && ./setup.sh && cd ..

# 2. Build project
make clean && make all

# 3. Run server
./bin/server

# 4. Run client (in new terminal)
./bin/client
```

---

## 📦 Class Reference

### Database
```cpp
#include "server/database.h"

auto db = std::make_shared<Database>(connString);
db->connect();
db->execute("INSERT INTO ...");
PGresult* res = db->query("SELECT * FROM ...");
PGresult* res = db->execParams(sql, nParams, paramValues);
```

### UserManager
```cpp
#include "server/user_manager.h"

auto um = std::make_shared<UserManager>(database);

// Authentication
bool valid = um->verifyCredentials(username, password);
bool exists = um->userExists(username);

// User info
int userId = um->getUserId(username);
string role = um->getUserRole(username);
string level = um->getUserLevel(username);

// Management
um->addUser(username, password, role, level);
um->updatePassword(username, newPassword);
um->updateUserLevel(username, newLevel);
```

### SessionManager
```cpp
#include "server/session.h"

auto sm = std::make_shared<SessionManager>(database, timeout);

// Session operations
string sessionId = sm->createSession(username, userId, clientFd);
bool valid = sm->validateSession(sessionId);
SessionData* session = sm->getSession(sessionId);
SessionData* session = sm->getSessionByFd(clientFd);

// Maintenance
sm->updateLastActive(sessionId);
sm->removeSession(sessionId);
sm->checkExpiredSessions();
```

---

## 🔌 Database Connection

### Default Connection String
```cpp
"host=localhost port=5432 dbname=english_learning user=postgres password=yourpass"
```

### Custom Connection
```cpp
#include "server/db_config.h"

string connStr = DBConfig::getConnectionString(
    "localhost",              // host
    5432,                     // port
    "english_learning",       // dbname
    "postgres",              // user
    "yourpass"               // password
);
```

---

## 🗄️ SQL Queries

### Users
```sql
-- Insert user
INSERT INTO users (username, password_hash, role, level) 
VALUES ($1, $2, $3, $4);

-- Get user
SELECT user_id, role, level 
FROM users 
WHERE username = $1;

-- Update password
UPDATE users 
SET password_hash = $1 
WHERE username = $2;
```

### Sessions
```sql
-- Create session
INSERT INTO server_sessions (session_id, user_id, client_fd) 
VALUES ($1, $2, $3);

-- Get active sessions
SELECT * FROM server_sessions 
WHERE active = TRUE;

-- Update last active
UPDATE server_sessions 
SET last_active = CURRENT_TIMESTAMP 
WHERE session_id = $1;

-- Deactivate session
UPDATE server_sessions 
SET active = FALSE 
WHERE session_id = $1;
```

---

## 🛠️ Build Commands

```bash
# Clean build
make clean

# Build all
make all

# Build server only
make bin/server

# Build client only
make bin/client

# Run targets
make run-server
make run-client

# Database test
cd database && make && ./db_test
```

---

## 🐛 Debugging

### Check Database Connection
```bash
psql -U postgres -d english_learning
```

### View Logs
```bash
tail -f logs/server.log
```

### List Tables
```sql
\dt
```

### Query Users
```sql
SELECT * FROM users;
```

### Query Sessions
```sql
SELECT s.session_id, u.username, s.active, s.last_active
FROM server_sessions s
JOIN users u ON s.user_id = u.user_id;
```

---

## ⚡ Common Tasks

### Add Test User
```cpp
userManager->addUser("testuser", "testpass", "student", "beginner");
```

### Reset Database
```bash
psql -U postgres -d english_learning -c "DROP SCHEMA public CASCADE; CREATE SCHEMA public;"
psql -U postgres -d english_learning -f database/init_db.sql
psql -U postgres -d english_learning -f database/seed_db.sql
```

### Change Database Password
```sql
ALTER USER postgres WITH PASSWORD 'newpassword';
```
Then update in:
- `include/server/db_config.h`
- `database/main.cpp`
- `src/server/server.cpp`

---

## 📋 Error Codes

### Database Errors
```
Connection failed          → Check PostgreSQL running
Authentication failed      → Check username/password
Database does not exist    → Run setup.sh
Permission denied          → Check user privileges
```

### Server Errors
```
Port already in use        → Kill process or change port
Database init failed       → Check connection string
Cannot open log file       → Check logs/ directory exists
```

---

## 🔐 Security Checklist

### Development
- [x] Parameterized queries
- [ ] Password hashing (TODO)
- [ ] Environment variables
- [ ] SSL/TLS connection

### Production
- [ ] Strong passwords
- [ ] Firewall rules
- [ ] Rate limiting
- [ ] Input validation
- [ ] Security audit
- [ ] Connection pooling
- [ ] Backup strategy

---

## 📞 Quick Help

### Database Setup Issues
See: `database/README.md`

### Integration Details
See: `POSTGRESQL_INTEGRATION.md`

### Full Documentation
See: `README.md`

### Code Examples
See: `database/main.cpp`

---

## 💡 Pro Tips

1. **Always** clear PGresult with PQclear()
2. **Use** parameterized queries for user input
3. **Check** database->isConnected() before operations
4. **Handle** nullptr returns from query methods
5. **Review** logs for database errors
6. **Test** with database connection failures
7. **Monitor** session cleanup runs every 5 seconds

---

## 🎯 Next Steps

1. ✅ Database setup complete
2. ✅ Integration tested
3. ⏭️ Add password hashing
4. ⏭️ Implement SSL/TLS
5. ⏭️ Add connection pooling
6. ⏭️ Production deployment

---

**Last Updated**: 2024  
**Quick Start Time**: ~5 minutes  
**Status**: Production Ready (with security TODOs)
