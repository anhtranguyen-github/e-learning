# How to Add New Features (Like Lessons) - Step by Step Guide

## 1. Plan Your Feature
- Define the feature's purpose and requirements
- Identify necessary database tables and relationships
- Plan the client-server communication protocol

## 2. Database Setup
- Create migration scripts for new tables (if needed)
- Add seed data for testing
- Update the database schema documentation

## 3. Backend Implementation

### a. Data Layer
- Create a data access class (e.g., [LessonLoader](cci:2://file:///mnt/c/Users/tra01/OneDrive/Desktop/socker/project/include/server/lesson_loader.h:102:0-120:1))
  - **Remember to create both [lesson_loader.h](cci:7://file:///mnt/c/Users/tra01/OneDrive/Desktop/socker/project/include/server/lesson_loader.h:0:0-0:0) and [lesson_loader.cpp](cci:7://file:///mnt/c/Users/tra01/OneDrive/Desktop/socker/project/src/server/lesson_loader.cpp:0:0-0:0)**
- Implement CRUD operations in both header and implementation files
- Add proper error handling and logging

### b. Business Logic
- Create a handler class (e.g., [LessonHandler](cci:2://file:///mnt/c/Users/tra01/OneDrive/Desktop/socker/project/include/server/lesson_handler.h:17:0-79:1))
  - **Create both [lesson_handler.h](cci:7://file:///mnt/c/Users/tra01/OneDrive/Desktop/socker/project/include/server/lesson_handler.h:0:0-0:0) and [lesson_handler.cpp](cci:7://file:///mnt/c/Users/tra01/OneDrive/Desktop/socker/project/src/server/lesson_handler.cpp:0:0-0:0)**
- Implement request processing logic in both files
- Add input validation

### c. Update Server
- Add new handler to the server
  - Update both [server.h](cci:7://file:///mnt/c/Users/tra01/OneDrive/Desktop/socker/project/include/server/server.h:0:0-0:0) (declaration) and [server.cpp](cci:7://file:///mnt/c/Users/tra01/OneDrive/Desktop/socker/project/src/server/server.cpp:0:0-0:0) (implementation)
- Register message handlers
- Update client handler to forward requests

## 4. Client Implementation
- Add new message types to protocol files
  - Update both header and implementation files
- Implement network layer methods
- Create UI components
- Connect UI to network layer

## 5. Testing
- Write unit tests in both header and implementation files
- Test both success and error cases
- Verify all new methods are properly tested

## 6. Documentation
- Update API documentation in header files
- Add usage examples in implementation files
- Document any new configuration options

## Key Reminders for .h and .cpp Files:
1. **For every class**:
   - Create/update the `.h` file with declarations
   - Create/update the corresponding `.cpp` file with implementations
   - Keep method signatures consistent between files

2. **When adding methods**:
   - Declare in `.h` file
   - Implement in `.cpp` file
   - Update documentation in both files

3. **For member variables**:
   - Declare in `.h` file
   - Initialize in constructor in `.cpp` file

4. **For includes**:
   - Forward declarations in `.h` when possible
   - Full includes in `.cpp`
   - Avoid circular dependencies

## Common Pitfalls to Avoid:
1. **Mismatched Signatures**: Ensure method signatures match exactly between `.h` and `.cpp` files
2. **Missing Includes**: If a method uses a type, ensure the header is included
3. **Forgotten Implementations**: Every declared method should have an implementation
4. **Incorrect Scope**: Don't forget the class name in method implementations

## Example Workflow for Adding a New Method:
1. Add method declaration to `.h` file
2. Add method implementation to `.cpp` file
3. Update any related methods in both files
4. Verify all includes are present in both files
5. Test the changes