# Client Design

## Overview
The client is a cross-platform desktop application built with Qt 6 and QML. It provides a rich, responsive user interface while maintaining a persistent TCP connection to the server.

## Core Components

### 1. NetworkClient (C++)
The low-level networking class.
-   **Socket Handling**: Manages the raw TCP socket (`sys/socket.h`).
-   **Protocol**: Serializes and deserializes `protocol::Message` objects.
-   **State**: Maintains connection state (`connected`, `loggedIn`, `sessionToken`, `userRole`).
-   **Blocking/Polling**: Uses a mix of blocking calls (for initial connect) and polling (via `NetworkManager` timer) for message reception.

### 2. NetworkManager (C++/Qt)
The bridge between C++ logic and QML UI.
-   **QObject**: Inherits from `QObject` to use Qt's Signal/Slot mechanism.
-   **Properties**: Exposes state to QML via `Q_PROPERTY` (e.g., `userRole`, `isConnected`).
-   **Invokables**: Exposes methods to QML (e.g., `login()`, `requestLessonList()`).
-   **Polling Timer**: A `QTimer` periodically calls `NetworkClient::pollMessages()` to check for incoming data without blocking the UI thread.

### 3. QML UI (View)
The presentation layer.
-   **StackView**: Manages navigation between screens (Login -> Dashboard -> Lessons).
-   **Components**: Reusable UI elements (Buttons, Lists, Dialogs).
-   **Models**: Uses `ListModel` to display dynamic data (e.g., `LessonModel`, `ChatModel`).

## Architecture Pattern: Model-View-Controller (MVC)

-   **Model**: C++ Data Structures (DTOs) and `NetworkClient` state.
-   **View**: QML files (`Dashboard.qml`, `LoginScreen.qml`).
-   **Controller**: `NetworkManager` mediates between View and Model.

## Data Flow Example: Login

1.  **User Input**: User enters credentials in `LoginScreen.qml`.
2.  **Action**: Button `onClicked` calls `networkManager.login(username, password)`.
3.  **Request**: `NetworkManager` calls `NetworkClient::login()`.
4.  **Network**: `NetworkClient` sends `LOGIN_REQUEST` and waits for response.
5.  **Response**: Server sends `LOGIN_SUCCESS` with `role`.
6.  **State Update**: `NetworkClient` updates `userRole`.
7.  **Signal**: `NetworkManager` emits `userRoleChanged` and `loginSuccess`.
8.  **UI Update**: QML listens for `onLoginSuccess` and switches to `Dashboard.qml`.

## Design Rationale

### Why QML?
-   **Declarative**: Makes building complex, animated UIs much faster than imperative C++ widgets.
-   **Separation**: Enforces separation of UI logic (JavaScript/QML) from Business logic (C++).

### Why Polling in NetworkManager?
-   **Simplicity**: Integrating a raw C++ socket into Qt's event loop (`QSocketNotifier`) is complex. A simple timer polling `pollMessages()` every 100ms is sufficient for this application's responsiveness requirements and much easier to implement and debug.
