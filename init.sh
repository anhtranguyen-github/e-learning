#!/bin/bash

# This script creates the directory structure for the TCP Learning App Project.

echo "Creating project directories..."

# Create top-level 'include' and 'src' directories with their subdirectories
mkdir -p include/client
mkdir -p include/common
mkdir -p include/server

mkdir -p src/client
mkdir -p src/common
mkdir -p src/server

echo "Creating header files (.h)..."

# Create header files in include/
touch include/client/network.h
touch include/client/ui.h

touch include/common/logger.h
touch include/common/protocol.h
touch include/common/utils.h

touch include/server/chat_handler.h
touch include/server/client_handler.h
touch include/server/database_utils.h
touch include/server/exercise_handlers.h
touch include/server/exercise_loader.h
touch include/server/lesson_handler.h
touch include/server/lesson_loader.h
touch include/server/server.h
touch include/server/session.h

echo "Creating source files (.cpp)..."

# Create source files in src/
touch src/client/client.cpp
touch src/client/network.cpp
touch src/client/ui.cpp

touch src/common/logger.cpp
touch src/common/protocol.cpp
touch src/common/utils.cpp

touch src/server/chat_handler.cpp
touch src/server/client_handler.cpp
touch src/server/database_utils.cpp
touch src/server/exercise_handlers.cpp
touch src/server/exercise_loader.cpp
touch src/server/lesson_handler.cpp
touch src/server/lesson_loader.cpp
touch src/server/server.cpp
# Note: session.cpp and database_utils.cpp might be needed, adding them based on the tree
touch src/server/session.cpp 

echo "✅ Project structure created successfully!"

# Display the final structure
tree .
