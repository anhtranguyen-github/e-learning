#include "server/session.h"
#include "common/utils.h"
#include <chrono>
#include <random>

namespace server {

SessionManager::SessionManager() : db_(nullptr) {}

SessionManager::SessionManager(std::shared_ptr<Database> db) : db_(db) {}

bool SessionManager::is_session_valid(const std::string& session_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return sessions_.count(session_id);
}

std::string SessionManager::create_session(int user_id, int client_fd) {
    std::string session_id = utils::generateSessionToken();
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_[session_id] = {user_id, std::chrono::steady_clock::now()};
    fd_to_session_id_[client_fd] = session_id;
    return session_id;
}

void SessionManager::remove_session(const std::string& session_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (sessions_.count(session_id)) {
        for (auto it = fd_to_session_id_.begin(); it != fd_to_session_id_.end(); ++it) {
            if (it->second == session_id) {
                fd_to_session_id_.erase(it);
                break;
            }
        }
        sessions_.erase(session_id);
    }
}

void SessionManager::update_session(const std::string& session_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (sessions_.count(session_id)) {
        sessions_[session_id].last_active = std::chrono::steady_clock::now();
    }
}

int SessionManager::get_user_id_by_session(const std::string& session_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (sessions_.count(session_id)) {
        return sessions_[session_id].user_id;
    }
    return -1;
}

int SessionManager::get_user_id_by_fd(int client_fd) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fd_to_session_id_.count(client_fd)) {
        std::string session_id = fd_to_session_id_[client_fd];
        if (sessions_.count(session_id)) {
            return sessions_[session_id].user_id;
        }
    }
    return -1;
}

void SessionManager::remove_session_by_fd(int client_fd) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fd_to_session_id_.count(client_fd)) {
        remove_session(fd_to_session_id_[client_fd]);
    }
}

std::vector<int> SessionManager::get_fds_by_user_id(int user_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<int> fds;
    for (const auto& pair : fd_to_session_id_) {
        int fd = pair.first;
        std::string session_id = pair.second;
        if (sessions_.count(session_id) && sessions_[session_id].user_id == user_id) {
            fds.push_back(fd);
        }
    }
    return fds;
}

} // namespace server
