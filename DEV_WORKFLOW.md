# Feature Development Workflow 

This document outlines the standard process for adding, updating, or refactoring features in the project, focusing on the key classes involved.

## 1. Define Data Contract (DTOs)
*   **File:** `include/common/payloads.h`
*   **Action:** Create or update `struct`s inheriting from `ISerializable`.
*   **Purpose:** Defines exactly what data is sent between Client and Server. This is the contract that both sides must adhere to.

## 2. Core Logic & Data (Model)
*   **Location:** `src/server/model/`
*   **Action:** Update classes (e.g., `Exam`, `Exercise`) to reflect new business rules or data structures.
*   **Purpose:** Encapsulates the "what" of your application. These classes represent the entities in your system.

## 3. Database Schema & Seeding
*   **Location:** `database/init_db.sql`, `database/seed.sql`
*   **Action:** Update `init_db.sql` for schema changes and `seed.sql` for initial data. **Note:** You must re-seed the database if the schema changes.
*   **Purpose:** Ensures the database structure matches your Model and has test data.

## 4. Data Persistence (Repository)
*   **Location:** `src/server/repository/`
*   **Action:** Update SQL queries in Repository classes to save/load the new Model data.
*   **Purpose:** Handles all direct database interactions. Keeps SQL out of your controllers.

## 5. Request Handling (Controller)
*   **Location:** `src/server/controller/`
*   **Action:** Implement logic in Controllers to process incoming requests, call Repositories, and return DTOs.
*   **Purpose:** The "brain" that connects the network request to the database. It orchestrates the flow.

## 6. Client Communication (NetworkManager)
*   **File:** `src/client/NetworkManager.cpp`
*   **Action:** Add methods to send requests and signals to receive responses.
*   **Purpose:** Bridges the UI with the Server. It handles the asynchronous nature of network calls.

## 7. User Interface (View)
*   **Location:** `src/client/gui/*.qml`
*   **Action:** Connect UI components to `NetworkManager` signals to display data.
*   **Purpose:** The visual layer the user interacts with. It should be dumb and only display data provided by the NetworkManager.

## 8. Build & Run
*   **File:** `./run.sh`
*   **Action:** Run this script to build the executable.
*   **Purpose:** Automates the build process for the project.
