#ifndef CLIENT_NETWORK_H
#define CLIENT_NETWORK_H

#include "common/protocol.h"
#include <string>
#include <vector>
#include <chrono>

namespace client {

class NetworkClient {
private:
    int sockfd;
    std::string serverHost;
    int serverPort;
    std::string sessionToken;
    bool connected;
    bool loggedIn;
    
    std::chrono::steady_clock::time_point lastHeartbeat;
    int heartbeatInterval; // seconds

    // Send raw data
    bool sendData(const std::vector<uint8_t>& data);
    
    // Receive raw data
    std::vector<uint8_t> receiveData();

public:
    NetworkClient(const std::string& host = "127.0.0.1", int port = 8080);
    ~NetworkClient();

    // Connection management
    bool connect();
    void disconnect();
    bool isConnected() const { return connected; }
    bool isLoggedIn() const { return loggedIn; }

    // Authentication
    bool login(const std::string& username, const std::string& password);
    bool logout();

    // Heartbeat
    bool sendHeartbeat();
    bool shouldSendHeartbeat();

    // Message sending/receiving
    bool sendMessage(const protocol::Message& msg);
    protocol::Message receiveMessage();

    // Session management
    std::string getSessionToken() const { return sessionToken; }
    void setSessionToken(const std::string& token) { sessionToken = token; }

    // Lesson management
    bool requestLessonList(const std::string& topic = "", const std::string& level = "");
    bool requestStudyLesson(int lessonId, const std::string& lessonType);

    // Socket file descriptor
    int getSocketFd() const { return sockfd; }
};

} // namespace client

#endif // CLIENT_NETWORK_H
