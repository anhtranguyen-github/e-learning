# Quick Start Guide

## Build the Project

```bash
make
```

This will create:
- `bin/server` - Server executable
- `bin/client` - Client executable

## Run the Server

Open a terminal and run:

```bash
./bin/server
```

You should see server logs indicating it's running on port 8080.

## Run the Client

Open another terminal and run:

```bash
./bin/client
```

## Test the Features

### 1. Connect to Server
- Choose option 1 from the client menu
- You should see "Connected successfully!"

### 2. Login
- Choose option 1 to login
- Use one of these default accounts:
  - Username: `admin`, Password: `admin123`
  - Username: `user1`, Password: `password1`
  - Username: `test`, Password: `test`

### 3. Observe Heartbeat
- After login, check the server logs
- You should see heartbeat messages every 10 seconds

### 4. View Status
- Choose option 2 to view your session status
- You'll see your session token

### 5. Logout
- Choose option 1 to logout
- Session will be cleared on both client and server

### 6. Test Session Expiration
- Login again
- Wait 30+ seconds without any activity
- The session will expire automatically
- Check server logs for expiration message

## Logs

Server logs: `logs/server.log`
Client logs: `logs/client.log`

View logs in real-time:
```bash
tail -f logs/server.log
tail -f logs/client.log
```

## Multiple Clients

You can run multiple clients simultaneously:

Terminal 1:
```bash
./bin/server
```

Terminal 2:
```bash
./bin/client
```

Terminal 3:
```bash
./bin/client
```

Each client can login with different users and maintain separate sessions.

## Clean Up

Stop server: Press Ctrl+C
Stop client: Choose exit option or press Ctrl+C

Clean build files:
```bash
make clean
```

Clean everything (including logs):
```bash
make clean-all
```
