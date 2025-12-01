# User Data Flow

```mermaid
sequenceDiagram
    participant Client
    participant Network as NetworkManager
    participant Controller as UserController
    participant Repo as UserRepository
    participant DB as Database
    participant Model as User
    participant DTO as UserDTO

    Note over Client, DB: Login Flow

    Client->>Network: send(LOGIN_REQUEST, "user;pass")
    Network->>Controller: handleUserLoginRequest(fd, msg)
    Controller->>Repo: verifyCredentials(username, password)
    Repo->>DB: SELECT password_hash FROM users...
    DB-->>Repo: true/false
    Repo-->>Controller: true

    Controller->>Repo: getUserId(username)
    Repo->>DB: SELECT id FROM users...
    DB-->>Repo: userId
    Repo-->>Controller: userId

    Controller->>Repo: findById(userId)
    Repo->>DB: SELECT * FROM users WHERE id = ?
    DB-->>Repo: Row
    Repo->>Model: User(id, username, role...)
    Model-->>Repo: User Instance
    Repo-->>Controller: User Instance

    Controller->>Controller: sessionMgr->create_session(...)
    Controller->>Controller: connMgr->add_client(...)

    Controller-->>Client: send(LOGIN_SUCCESS, "session_id;role")
```
