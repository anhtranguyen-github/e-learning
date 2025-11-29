#ifndef SESSION_H
#define SESSION_H

#include "server/database.h"
#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <vector>
#include <memory>

namespace server {

class SessionManager {
public:
    SessionManager();
    SessionManager(std::shared_ptr<Database> db);

    bool is_session_valid(const std::string& session_id);
    std::string create_session(int user_id, int client_fd);
    void remove_session(const std::string& session_id);
    void update_session(const std::string& session_id);
    int get_user_id_by_session(const std::string& session_id);
    int get_user_id_by_fd(int client_fd);
    std::vector<int> get_fds_by_user_id(int user_id);
    void remove_session_by_fd(int client_fd);

private:
    struct Session {
        int user_id;
        std::chrono::steady_clock::time_point last_active;
    };

    std::unordered_map<std::string, Session> sessions_;
    std::unordered_map<int, std::string> fd_to_session_id_;
    std::shared_ptr<Database> db_;
    std::mutex mutex_;
};

} // namespace server

#endif // SESSION_H
