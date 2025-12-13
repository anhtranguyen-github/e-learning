# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include -I/usr/include/jsoncpp -pthread
LDFLAGS = -pthread -lpq -ljsoncpp

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Source files
COMMON_SRC = $(SRC_DIR)/common/logger.cpp $(SRC_DIR)/common/utils.cpp $(SRC_DIR)/common/protocol.cpp


SERVER_SRC = $(SRC_DIR)/server/server.cpp \
             $(SRC_DIR)/server/database.cpp \
             $(SRC_DIR)/server/connection_manager.cpp \
             $(SRC_DIR)/server/request_router.cpp \
             $(SRC_DIR)/server/client_handler.cpp \
             $(SRC_DIR)/server/session.cpp \
             $(SRC_DIR)/server/model/lesson.cpp \
             $(SRC_DIR)/server/model/exercise.cpp \
             $(SRC_DIR)/server/model/exam.cpp \
             $(SRC_DIR)/server/model/user.cpp \
             $(SRC_DIR)/server/model/game.cpp \
             $(SRC_DIR)/server/repository/lesson_repository.cpp \
             $(SRC_DIR)/server/repository/exercise_repository.cpp \
             $(SRC_DIR)/server/repository/exam_repository.cpp \
             $(SRC_DIR)/server/repository/user_repository.cpp \
             $(SRC_DIR)/server/repository/result_repository.cpp \
             $(SRC_DIR)/server/repository/chat_repository.cpp \
             $(SRC_DIR)/server/repository/game_repository.cpp \
             $(SRC_DIR)/server/controller/lesson_controller.cpp \
             $(SRC_DIR)/server/controller/exercise_controller.cpp \
             $(SRC_DIR)/server/controller/submission_controller.cpp \
             $(SRC_DIR)/server/controller/result_controller.cpp \
             $(SRC_DIR)/server/controller/student_exam_controller.cpp \
             $(SRC_DIR)/server/controller/teacher_exam_controller.cpp \
             $(SRC_DIR)/server/controller/chat_controller.cpp \
             $(SRC_DIR)/server/controller/feedback_controller.cpp \
             $(SRC_DIR)/server/controller/game_controller.cpp \
             $(SRC_DIR)/server/controller/admin_game_controller.cpp \
             $(SRC_DIR)/server/controller/user_controller.cpp


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
	@mkdir -p $(BUILD_DIR)/server/model
	@mkdir -p $(BUILD_DIR)/server/repository
	@mkdir -p $(BUILD_DIR)/server/controller
	@mkdir -p $(BUILD_DIR)/client
	@mkdir -p $(BIN_DIR)
	@mkdir -p logs
	@mkdir -p data

# Server executable
$(SERVER_BIN): $(COMMON_OBJ) $(SERVER_OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Client executable
$(CLIENT_BIN): $(COMMON_OBJ) $(CLIENT_OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

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
