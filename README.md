## Compilation and Running Instructions

This project consists of a C++ TCP server and client. You will need a C++ compiler (like g++ on Linux/macOS or MinGW g++ on Windows, or MSVC on Windows).

### 1. Create `logs` directory
Both the client and server will attempt to create a `logs` directory in their respective execution paths. Ensure you have write permissions in the directory where you run the executables.

### 2. Compile the Server

**On Linux/macOS (using g++):**

```bash
g++ -std=c++17 -pthread -o server \
    src/common/logger.cpp \
    src/common/protocol.cpp \
    src/server/session.cpp \
    src/server/server.cpp \
    -Iinclude
```

**On Windows (using MinGW g++):**

```bash
g++ -std=c++17 -pthread -o server.exe \
    src/common/logger.cpp \
    src/common/protocol.cpp \
    src/server/session.cpp \
    src/server/server.cpp \
    -Iinclude -lws2_32
```

**On Windows (using MSVC - Developer Command Prompt):**

```cmd
cl /std:c++17 /EHsc /Fo:server.obj /Fe:server.exe \
    src/common/logger.cpp \
    src/common/protocol.cpp \
    src/server/session.cpp \
    src/server/server.cpp \
    /Iinclude Ws2_32.lib
```

### 3. Compile the Client

**On Linux/macOS (using g++):**

```bash
g++ -std=c++17 -pthread -o client \
    src/common/logger.cpp \
    src/common/protocol.cpp \
    src/client/network.cpp \
    src/client/ui.cpp \
    src/client/client.cpp \
    -Iinclude
```

**On Windows (using MinGW g++):**

```bash
g++ -std=c++17 -pthread -o client.exe \
    src/common/logger.cpp \
    src/common/protocol.cpp \
    src/client/network.cpp \
    src/client/ui.cpp \
    src/client/client.cpp \
    -Iinclude -lws2_32
```

**On Windows (using MSVC - Developer Command Prompt):**

```cmd
cl /std:c++17 /EHsc /Fo:client.obj /Fe:client.exe \
    src/common/logger.cpp \
    src/common/protocol.cpp \
    src/client/network.cpp \
    src/client/ui.cpp \
    src/client/client.cpp \
    /Iinclude Ws2_32.lib
```

### 4. Run the Applications

**First, start the server:**

```bash
./server
```
(or `server.exe` on Windows)

**Then, in a separate terminal, start the client:**

```bash
./client
```
(or `client.exe` on Windows)

### 5. Client Commands

Once the client is running, you can type commands at the `>` prompt:

*   `login <username> <password>`: e.g., `login user1 pass1`
*   `logout`
*   `heartbeat`
*   `disconnect`
*   `quit` or `exit`

**Note:** The `users.txt` file should be in the same directory where you run the server executable. The default content is:

```
user1:pass1
user2:pass2
```

This completes the initial implementation based on your instructions. You can now compile and test the client-server communication.
