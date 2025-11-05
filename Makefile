# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include -pthread
LDFLAGS = -pthread

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Source files
COMMON_SRC = $(SRC_DIR)/common/logger.cpp $(SRC_DIR)/common/utils.cpp
SERVER_SRC = $(SRC_DIR)/server/server.cpp $(SRC_DIR)/server/session.cpp \
             $(SRC_DIR)/server/database_utils.cpp $(SRC_DIR)/server/client_handler.cpp
CLIENT_SRC = $(SRC_DIR)/client/client.cpp $(SRC_DIR)/client/network.cpp \
             $(SRC_DIR)/client/ui.cpp

# Object files
COMMON_OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(COMMON_SRC))
SERVER_OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SERVER_SRC))
CLIENT_OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CLIENT_SRC))

# Executables
SERVER_BIN = $(BIN_DIR)/server
CLIENT_BIN = $(BIN_DIR)/client

# Default target
all: directories $(SERVER_BIN) $(CLIENT_BIN)

# Create necessary directories
directories:
	@mkdir -p $(BUILD_DIR)/common
	@mkdir -p $(BUILD_DIR)/server
	@mkdir -p $(BUILD_DIR)/client
	@mkdir -p $(BIN_DIR)
	@mkdir -p logs
	@mkdir -p data

# Server executable
$(SERVER_BIN): $(COMMON_OBJ) $(SERVER_OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

# Client executable
$(CLIENT_BIN): $(COMMON_OBJ) $(CLIENT_OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

# Compile common source files
$(BUILD_DIR)/common/%.o: $(SRC_DIR)/common/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile server source files
$(BUILD_DIR)/server/%.o: $(SRC_DIR)/server/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile client source files
$(BUILD_DIR)/client/%.o: $(SRC_DIR)/client/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Clean logs and data
clean-all: clean
	rm -rf logs data

# Run server
run-server: $(SERVER_BIN)
	./$(SERVER_BIN)

# Run client
run-client: $(CLIENT_BIN)
	./$(CLIENT_BIN)

.PHONY: all directories clean clean-all run-server run-client
