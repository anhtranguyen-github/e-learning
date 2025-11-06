#ifndef SESSION_H
#define SESSION_H

#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <vector>

namespace server {

struct SessionData {
    std::string sessionId;
    std::string username;
    int clientFd;
    std::chrono::steady_clock::time_point lastActive;
    bool active;

    SessionData() : clientFd(-1), active(false) {}
    
    SessionData(const std::string& sid, const std::string& user, int fd)
        : sessionId(sid), username(user), clientFd(fd), 
          lastActive(std::chrono::steady_clock::now()), active(true) {}
};

class SessionManager {
private:
    std::unordered_map<std::string, SessionData> sessions; // sessionId -> SessionData
    std::unordered_map<int, std::string> fdToSession;      // clientFd -> sessionId
    std::mutex sessionMutex;
    int heartbeatTimeout; // seconds

public:
    SessionManager(int timeout = 30);

    // Create a new session
    std::string createSession(const std::string& username, int clientFd);

    // Validate session token
    bool validateSession(const std::string& sessionId);

    // Get session data
    SessionData* getSession(const std::string& sessionId);
    SessionData* getSessionByFd(int clientFd);

    // Update last active time (for heartbeat)
    void updateLastActive(const std::string& sessionId);

    // Remove session
    void removeSession(const std::string& sessionId);
    void removeSessionByFd(int clientFd);

    // Check for expired sessions
    void checkExpiredSessions();

    // Get all active sessions
    std::vector<SessionData> getActiveSessions();
};

} // namespace server

#endif // SESSION_H
