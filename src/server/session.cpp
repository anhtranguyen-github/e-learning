#include "server/session.h"
#include "common/utils.h"
#include "common/logger.h"
#include <algorithm>
#include <sstream>

namespace server {

SessionManager::SessionManager(std::shared_ptr<Database> database, int timeout) 
    : db(database), heartbeatTimeout(timeout) {}

std::string SessionManager::createSession(const std::string& username, int userId, int clientFd) {
    std::lock_guard<std::mutex> lock(sessionMutex);
    
    std::string sessionId = utils::generateSessionToken();
    SessionData session(sessionId, username, userId, clientFd);
    
    sessions[sessionId] = session;
    fdToSession[clientFd] = sessionId;
    
    // Persist to database
    saveSessionToDB(session);
    
    if (logger::serverLogger) {
        logger::serverLogger->info("Created session " + sessionId + 
                                  " for user " + username + 
                                  " (fd=" + std::to_string(clientFd) + ")");
    }
    
    return sessionId;
}

bool SessionManager::validateSession(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(sessionMutex);
    
    auto it = sessions.find(sessionId);
    if (it == sessions.end()) {
        return false;
    }
    
    return it->second.active;
}

SessionData* SessionManager::getSession(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(sessionMutex);
    
    auto it = sessions.find(sessionId);
    if (it == sessions.end()) {
        return nullptr;
    }
    
    return &(it->second);
}

SessionData* SessionManager::getSessionByFd(int clientFd) {
    std::lock_guard<std::mutex> lock(sessionMutex);
    
    auto it = fdToSession.find(clientFd);
    if (it == fdToSession.end()) {
        return nullptr;
    }
    
    return getSession(it->second);
}

void SessionManager::updateLastActive(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(sessionMutex);
    
    auto it = sessions.find(sessionId);
    if (it != sessions.end()) {
        it->second.lastActive = std::chrono::steady_clock::now();
        
        // Update in database
        updateSessionInDB(sessionId);
        
        if (logger::serverLogger) {
            logger::serverLogger->debug("Updated last active for session " + sessionId);
        }
    }
}

void SessionManager::removeSession(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(sessionMutex);
    
    auto it = sessions.find(sessionId);
    if (it != sessions.end()) {
        int fd = it->second.clientFd;
        fdToSession.erase(fd);
        sessions.erase(it);
        
        // Remove from database
        deleteSessionFromDB(sessionId);
        
        if (logger::serverLogger) {
            logger::serverLogger->info("Removed session " + sessionId);
        }
    }
}

void SessionManager::removeSessionByFd(int clientFd) {
    std::lock_guard<std::mutex> lock(sessionMutex);
    
    auto it = fdToSession.find(clientFd);
    if (it != fdToSession.end()) {
        std::string sessionId = it->second;
        
        // Remove from database
        deleteSessionFromDB(sessionId);
        
        sessions.erase(sessionId);
        fdToSession.erase(it);
        
        if (logger::serverLogger) {
            logger::serverLogger->info("Removed session for fd=" + std::to_string(clientFd));
        }
    }
}

void SessionManager::checkExpiredSessions() {
    std::lock_guard<std::mutex> lock(sessionMutex);
    
    auto now = std::chrono::steady_clock::now();
    std::vector<std::string> expiredSessions;
    
    for (auto& pair : sessions) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - pair.second.lastActive).count();
        
        if (elapsed > heartbeatTimeout) {
            expiredSessions.push_back(pair.first);
            
            if (logger::serverLogger) {
                logger::serverLogger->warn("Session " + pair.first + 
                                          " expired (timeout=" + 
                                          std::to_string(elapsed) + "s)");
            }
        }
    }
    
    // Remove expired sessions
    for (const auto& sessionId : expiredSessions) {
        auto it = sessions.find(sessionId);
        if (it != sessions.end()) {
            fdToSession.erase(it->second.clientFd);
            sessions.erase(it);
        }
    }
}

std::vector<SessionData> SessionManager::getActiveSessions() {
    std::lock_guard<std::mutex> lock(sessionMutex);
    
    std::vector<SessionData> activeSessions;
    for (const auto& pair : sessions) {
        if (pair.second.active) {
            activeSessions.push_back(pair.second);
        }
    }
    
    return activeSessions;
}

// Database persistence methods
bool SessionManager::saveSessionToDB(const SessionData& session) {
    if (!db || !db->isConnected()) {
        return false;
    }
    
    std::string userIdStr = std::to_string(session.userId);
    std::string clientFdStr = std::to_string(session.clientFd);
    
    const char* paramValues[3] = {
        session.sessionId.c_str(),
        userIdStr.c_str(),
        clientFdStr.c_str()
    };
    
    PGresult* res = db->execParams(
        "INSERT INTO server_sessions (session_id, user_id, client_fd, last_active, active) "
        "VALUES ($1, $2, $3, CURRENT_TIMESTAMP, TRUE) "
        "ON CONFLICT (session_id) DO UPDATE SET "
        "client_fd = EXCLUDED.client_fd, last_active = CURRENT_TIMESTAMP, active = TRUE",
        3, paramValues
    );
    
    if (res) {
        PQclear(res);
        return true;
    }
    return false;
}

bool SessionManager::loadSessionFromDB(const std::string& sessionId) {
    if (!db || !db->isConnected()) {
        return false;
    }
    
    const char* paramValues[1] = {sessionId.c_str()};
    
    PGresult* res = db->execParams(
        "SELECT user_id, client_fd, active FROM server_sessions WHERE session_id = $1",
        1, paramValues
    );
    
    if (!res || PQntuples(res) == 0) {
        if (res) PQclear(res);
        return false;
    }
    
    // Note: This is a simplified version. Full implementation would reconstruct SessionData
    // from database including username lookup
    
    PQclear(res);
    return true;
}

bool SessionManager::updateSessionInDB(const std::string& sessionId) {
    if (!db || !db->isConnected()) {
        return false;
    }
    
    const char* paramValues[1] = {sessionId.c_str()};
    
    PGresult* res = db->execParams(
        "UPDATE server_sessions SET last_active = CURRENT_TIMESTAMP WHERE session_id = $1",
        1, paramValues
    );
    
    if (res) {
        PQclear(res);
        return true;
    }
    return false;
}

bool SessionManager::deleteSessionFromDB(const std::string& sessionId) {
    if (!db || !db->isConnected()) {
        return false;
    }
    
    const char* paramValues[1] = {sessionId.c_str()};
    
    PGresult* res = db->execParams(
        "UPDATE server_sessions SET active = FALSE WHERE session_id = $1",
        1, paramValues
    );
    
    if (res) {
        PQclear(res);
        return true;
    }
    return false;
}

} // namespace server
