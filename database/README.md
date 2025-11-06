# Database Setup Guide

## PostgreSQL Integration

This project uses PostgreSQL for persistent data storage including user management and session management.

## Prerequisites

1. **Install PostgreSQL**
   ```bash
   sudo apt-get update
   sudo apt-get install postgresql postgresql-contrib libpq-dev
   ```

2. **Start PostgreSQL Service**
   ```bash
   sudo service postgresql start
   ```

## Database Setup

1. **Create Database and User**
   ```bash
   sudo -u postgres psql
   ```
   
   In psql console:
   ```sql
   CREATE DATABASE english_learning;
   CREATE USER postgres WITH PASSWORD 'yourpass';
   GRANT ALL PRIVILEGES ON DATABASE english_learning TO postgres;
   \q
   ```

2. **Initialize Database Schema**
   ```bash
   psql -U postgres -d english_learning -f init_db.sql
   ```

3. **Seed Database with Test Data** (Optional)
   ```bash
   psql -U postgres -d english_learning -f seed_db.sql
   ```

## Database Structure

### Tables

- **users**: User accounts with authentication
- **lessons**: Learning content
- **exercises**: Practice exercises
- **exams**: Assessment tests
- **results**: User performance tracking
- **game_items**: Gamification elements
- **server_sessions**: Active user sessions

## Testing Database Connection

Build and run the test program:

```bash
cd database
make
./db_test
```

## Connection Configuration

Default connection string:
```
host=localhost port=5432 dbname=english_learning user=postgres password=yourpass
```

Update password in:
- `include/server/db_config.h`
- `database/main.cpp`
- `src/server/server.cpp`

## Server Integration

The server now uses:
- **Database**: PostgreSQL connection manager
- **UserManager**: User authentication and management
- **SessionManager**: Session persistence in database

All user data and sessions are now stored in PostgreSQL instead of flat files.

## Cleanup

To reset the database:
```bash
psql -U postgres -d english_learning -c "DROP SCHEMA public CASCADE; CREATE SCHEMA public;"
psql -U postgres -d english_learning -f init_db.sql
```
