#include "server/session.h"
#include "common/utils.h"
#include "common/logger.h"
#include <algorithm>

namespace server {

SessionManager::SessionManager(int timeout) : heartbeatTimeout(timeout) {}

std::string SessionManager::createSession(const std::string& username, int clientFd) {
    std::lock_guard<std::mutex> lock(sessionMutex);
    
    std::string sessionId = utils::generateSessionToken();
    SessionData session(sessionId, username, clientFd);
    
    sessions[sessionId] = session;
    fdToSession[clientFd] = sessionId;
    
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

} // namespace server
