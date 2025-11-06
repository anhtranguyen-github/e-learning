# Makefile Update Guide for Lesson Management

## Required Changes

Add the following to your existing Makefile to compile the lesson management features:

### 1. Add Lesson Object Files

```makefile
# Server object files
SERVER_OBJS = \
    src/server/server.o \
    src/server/database.o \
    src/server/session.o \
    src/server/user_manager.o \
    src/server/client_handler.o \
    src/server/lesson_loader.o \
    src/server/lesson_handler.o \
    src/common/protocol.o \
    src/common/logger.o \
    src/common/utils.o
```

### 2. Add Required Libraries

```makefile
# Libraries
LIBS = -lpq -ljsoncpp -lpthread
```

**Required libraries:**
- `-lpq`: PostgreSQL client library
- `-ljsoncpp`: JSON parsing library (for JSONB fields)
- `-lpthread`: POSIX threads (for heartbeat thread)

### 3. Complete Makefile Example

```makefile
# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
LIBS = -lpq -ljsoncpp -lpthread

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

# Common source files
COMMON_OBJS = \
    $(BUILD_DIR)/common/protocol.o \
    $(BUILD_DIR)/common/logger.o \
    $(BUILD_DIR)/common/utils.o

# Server source files
SERVER_OBJS = \
    $(BUILD_DIR)/server/server.o \
    $(BUILD_DIR)/server/database.o \
    $(BUILD_DIR)/server/session.o \
    $(BUILD_DIR)/server/user_manager.o \
    $(BUILD_DIR)/server/client_handler.o \
    $(BUILD_DIR)/server/lesson_loader.o \
    $(BUILD_DIR)/server/lesson_handler.o

# Client source files
CLIENT_OBJS = \
    $(BUILD_DIR)/client/client.o \
    $(BUILD_DIR)/client/network.o \
    $(BUILD_DIR)/client/ui.o

# Executables
SERVER_BIN = server
CLIENT_BIN = client

# Default target
all: $(SERVER_BIN) $(CLIENT_BIN)

# Server executable
$(SERVER_BIN): $(SERVER_OBJS) $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo "Server built successfully!"

# Client executable
$(CLIENT_BIN): $(CLIENT_OBJS) $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo "Client built successfully!"

# Build common objects
$(BUILD_DIR)/common/%.o: $(SRC_DIR)/common/%.cpp
	@mkdir -p $(BUILD_DIR)/common
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build server objects
$(BUILD_DIR)/server/%.o: $(SRC_DIR)/server/%.cpp
	@mkdir -p $(BUILD_DIR)/server
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build client objects
$(BUILD_DIR)/client/%.o: $(SRC_DIR)/client/%.cpp
	@mkdir -p $(BUILD_DIR)/client
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(SERVER_BIN) $(CLIENT_BIN)
	@echo "Clean complete!"

# Rebuild everything
rebuild: clean all

# Install dependencies (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y \
		g++ \
		make \
		libpq-dev \
		libjsoncpp-dev \
		postgresql-client
	@echo "Dependencies installed!"

# Run server
run-server: $(SERVER_BIN)
	./$(SERVER_BIN)

# Run client
run-client: $(CLIENT_BIN)
	./$(CLIENT_BIN)

.PHONY: all clean rebuild install-deps run-server run-client
```

## Installation of Dependencies

### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install -y g++ make libpq-dev libjsoncpp-dev postgresql-client
```

### CentOS/RHEL:
```bash
sudo yum install -y gcc-c++ make postgresql-devel jsoncpp-devel
```

### macOS (with Homebrew):
```bash
brew install gcc make postgresql jsoncpp
```

## Build Commands

```bash
# Build everything
make all

# Build server only
make server

# Build client only
make client

# Clean and rebuild
make rebuild

# Clean build artifacts
make clean

# Install dependencies (Ubuntu/Debian)
make install-deps
```

## Verify Compilation

After building, verify the executables:

```bash
# Check server
./server --help

# Check client
./client --help

# Check linked libraries
ldd server | grep -E 'pq|json'
ldd client | grep -E 'pq|json'
```

Expected output should show:
- `libpq.so` (PostgreSQL)
- `libjsoncpp.so` (JSON parsing)
- `libpthread.so` (Threads)

## Troubleshooting

### Error: "libpq.so not found"
```bash
# Find PostgreSQL library path
sudo find /usr -name "libpq.so*"

# Add to LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/lib/postgresql/XX/lib:$LD_LIBRARY_PATH

# Or install PostgreSQL development files
sudo apt-get install libpq-dev
```

### Error: "jsoncpp/json.h not found"
```bash
# Install JsonCpp development files
sudo apt-get install libjsoncpp-dev

# Or use different include path
CXXFLAGS += -I/usr/include/jsoncpp
```

### Error: "undefined reference to PQconnectdb"
```bash
# Make sure -lpq is in LIBS
LIBS = -lpq -ljsoncpp -lpthread

# Link order matters, put -lpq before object files
```

## File Structure

After building, your project structure should be:

```
project/
├── build/                  # Compiled object files
│   ├── common/
│   │   ├── protocol.o
│   │   ├── logger.o
│   │   └── utils.o
│   ├── server/
│   │   ├── server.o
│   │   ├── database.o
│   │   ├── session.o
│   │   ├── user_manager.o
│   │   ├── client_handler.o
│   │   ├── lesson_loader.o      # NEW
│   │   └── lesson_handler.o     # NEW
│   └── client/
│       ├── client.o
│       ├── network.o
│       └── ui.o
├── server                  # Server executable
├── client                  # Client executable
└── Makefile
```

## Quick Build Test

```bash
# Clean and rebuild with verbose output
make clean
make CXXFLAGS="-std=c++17 -Wall -Wextra -Iinclude -v" all 2>&1 | tee build.log

# Check for errors
grep -i error build.log

# Check for warnings
grep -i warning build.log
```

## Performance Optimization

For production builds, add optimization flags:

```makefile
# Release build with optimizations
CXXFLAGS_RELEASE = -std=c++17 -O3 -DNDEBUG -Iinclude

# Debug build with symbols
CXXFLAGS_DEBUG = -std=c++17 -g -O0 -DDEBUG -Iinclude

# Default to release
CXXFLAGS ?= $(CXXFLAGS_RELEASE)
```

## Summary

Key changes needed:
1. ✅ Add `lesson_loader.o` and `lesson_handler.o` to server objects
2. ✅ Add `-ljsoncpp` to libraries (for JSON parsing)
3. ✅ Ensure `-lpq` is included (for PostgreSQL)
4. ✅ Keep `-lpthread` for threading support

The lesson management system is now ready to compile!
