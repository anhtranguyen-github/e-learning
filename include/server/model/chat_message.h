#ifndef SERVER_MODEL_CHAT_MESSAGE_H
#define SERVER_MODEL_CHAT_MESSAGE_H

#include <string>

namespace server {

class ChatMessage {
private:
    int id;
    int senderId;
    int receiverId;
    std::string content;
    std::string messageType;
    std::string timestamp;
    bool isRead;

public:
    ChatMessage() : id(-1), senderId(-1), receiverId(-1), isRead(false) {}
    ChatMessage(int id, int senderId, int receiverId, const std::string& content, const std::string& messageType, const std::string& timestamp, bool isRead)
        : id(id), senderId(senderId), receiverId(receiverId), content(content), messageType(messageType), timestamp(timestamp), isRead(isRead) {}

    int getId() const { return id; }
    int getSenderId() const { return senderId; }
    int getReceiverId() const { return receiverId; }
    std::string getContent() const { return content; }
    std::string getMessageType() const { return messageType; }
    std::string getTimestamp() const { return timestamp; }
    bool getIsRead() const { return isRead; }

    void setId(int id) { this->id = id; }
    void setSenderId(int senderId) { this->senderId = senderId; }
    void setReceiverId(int receiverId) { this->receiverId = receiverId; }
    void setContent(const std::string& content) { this->content = content; }
    void setMessageType(const std::string& messageType) { this->messageType = messageType; }
    void setTimestamp(const std::string& timestamp) { this->timestamp = timestamp; }
    void setIsRead(bool isRead) { this->isRead = isRead; }
};

} // namespace server

#endif // SERVER_MODEL_CHAT_MESSAGE_H
