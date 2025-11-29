#include "server/controller/chat_controller.h"
#include "common/logger.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <sys/socket.h>

namespace server {

ChatController::ChatController(std::shared_ptr<ChatRepository> chatRepo,
                               std::shared_ptr<UserRepository> userRepo,
                               std::shared_ptr<ConnectionManager> connMgr,
                               std::shared_ptr<SessionManager> sessionMgr)
    : chatRepository(chatRepo), userRepository(userRepo), connectionManager(connMgr), sessionManager(sessionMgr) {}

void ChatController::handleUserSendPrivateMessage(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("handleSendPrivateMessage: Raw payload=" + payload);
    }

    Payloads::PrivateMessageRequest req;
    try {
        req.deserialize(payload);
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("handleSendPrivateMessage: Deserialization failed: " + std::string(e.what()));
        }
        protocol::Message response(protocol::MsgCode::CHAT_MESSAGE_FAILURE, "Invalid message format");
        std::vector<uint8_t> data = response.serialize();
        send(clientFd, data.data(), data.size(), 0);
        return;
    }

    if (logger::serverLogger) {
        logger::serverLogger->debug("handleSendPrivateMessage: recipient=" + req.recipient + ", content=" + req.content);
    }

    int senderId = sessionManager->get_user_id_by_session(req.sessionToken);
    if (senderId == -1) {
        if (logger::serverLogger) logger::serverLogger->warn("handleSendPrivateMessage: Invalid session token");
        protocol::Message response(protocol::MsgCode::CHAT_MESSAGE_FAILURE, "Invalid session");
        std::vector<uint8_t> data = response.serialize();
        send(clientFd, data.data(), data.size(), 0);
        return;
    }

    User sender = userRepository->findById(senderId);
    if (sender.getId() == -1) {
        if (logger::serverLogger) logger::serverLogger->warn("handleSendPrivateMessage: Sender not found for ID " + std::to_string(senderId));
        protocol::Message response(protocol::MsgCode::CHAT_MESSAGE_FAILURE, "Sender not found");
        std::vector<uint8_t> data = response.serialize();
        send(clientFd, data.data(), data.size(), 0);
        return;
    }

    int receiverId = userRepository->getUserId(req.recipient);
    if (receiverId == -1) {
        if (logger::serverLogger) logger::serverLogger->warn("handleSendPrivateMessage: Recipient not found: " + req.recipient);
        protocol::Message response(protocol::MsgCode::CHAT_MESSAGE_FAILURE, "Recipient not found");
        std::vector<uint8_t> data = response.serialize();
        send(clientFd, data.data(), data.size(), 0);
        return;
    }
    User receiver = userRepository->findById(receiverId);

    if (logger::serverLogger) {
        logger::serverLogger->debug("Handling send message: senderId=" + std::to_string(senderId) + 
                                   ", receiverId=" + std::to_string(receiverId) + 
                                   ", content=" + req.content);
    }

    // Create ChatMessage object
    ChatMessage chatMsg;
    chatMsg.setSenderId(senderId);
    chatMsg.setReceiverId(receiverId);
    chatMsg.setContent(req.content);
    chatMsg.setMessageType(req.messageType.empty() ? "TEXT" : req.messageType);
    chatMsg.setIsRead(false);

    // Save to DB
    int msgId = chatRepository->saveMessage(chatMsg);
    if (msgId == -1) {
        protocol::Message response(protocol::MsgCode::CHAT_MESSAGE_FAILURE, "Failed to save message");
        std::vector<uint8_t> data = response.serialize();
        send(clientFd, data.data(), data.size(), 0);
        return;
    }

    // Get timestamp
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
    std::string timestamp = ss.str();

    // Prepare push notification payload
    Payloads::ChatMessageDTO pushDto;
    pushDto.sender = sender.getUsername();
    pushDto.messageType = chatMsg.getMessageType();
    pushDto.content = chatMsg.getContent();
    pushDto.timestamp = timestamp;
    
    std::string pushPayload = pushDto.serialize();
    protocol::Message pushMsg(protocol::MsgCode::CHAT_PRIVATE_RECEIVE, pushPayload);

    // Send to recipient if online
    connectionManager->sendToUser(receiverId, pushMsg);

    // Send success to sender
    protocol::Message response(protocol::MsgCode::CHAT_MESSAGE_SUCCESS, "Message sent");
    std::vector<uint8_t> data = response.serialize();
    send(clientFd, data.data(), data.size(), 0);
}

void ChatController::handleUserGetChatHistory(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    Payloads::ChatHistoryRequest req;
    req.deserialize(payload);

    int userId1 = sessionManager->get_user_id_by_session(req.sessionToken);
    if (userId1 == -1) {
        protocol::Message response(protocol::MsgCode::CHAT_HISTORY_FAILURE, "Invalid session");
        std::vector<uint8_t> data = response.serialize();
        send(clientFd, data.data(), data.size(), 0);
        return;
    }

    int userId2 = userRepository->getUserId(req.otherUser);
    if (userId2 == -1) {
        protocol::Message response(protocol::MsgCode::CHAT_HISTORY_FAILURE, "User not found");
        std::vector<uint8_t> data = response.serialize();
        send(clientFd, data.data(), data.size(), 0);
        return;
    }

    std::vector<ChatMessage> history = chatRepository->getChatHistory(userId1, userId2);
    
    // Convert to DTOs
    Payloads::ChatHistoryDTO historyDto;
    for (const auto& m : history) {
        Payloads::ChatMessageDTO dto;
        if (m.getSenderId() == userId1) {
            User user = userRepository->findById(userId1);
            dto.sender = (user.getId() != -1) ? user.getUsername() : "Unknown";
        } else {
            dto.sender = req.otherUser;
        }
        
        dto.messageType = m.getMessageType();
        dto.content = m.getContent();
        dto.timestamp = m.getTimestamp();
        historyDto.messages.push_back(dto);
    }

    protocol::Message response(protocol::MsgCode::CHAT_HISTORY_SUCCESS, historyDto.serialize());
    std::vector<uint8_t> data = response.serialize();
    send(clientFd, data.data(), data.size(), 0);
}

void ChatController::handleUserGetRecentChats(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    Payloads::RecentChatsRequest req;
    req.deserialize(payload);

    int userId = sessionManager->get_user_id_by_session(req.sessionToken);
    if (userId == -1) {
        protocol::Message response(protocol::MsgCode::RECENT_CHATS_FAILURE, "Invalid session");
        std::vector<uint8_t> data = response.serialize();
        send(clientFd, data.data(), data.size(), 0);
        return;
    }

    std::vector<ChatMessage> recentChats = chatRepository->getRecentChats(userId);
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("getRecentChats returned " + std::to_string(recentChats.size()) + " messages for userId=" + std::to_string(userId));
    }

    // Convert to DTOs
    // We need a DTO that holds a list of recent chats summaries.
    // Since we don't have a specific RecentChatsDTO, we can reuse a string format or create a DTO.
    // The client expects "userId;username;lastMessage;timestamp|..."
    
    std::stringstream ss;
    for (const auto& m : recentChats) {
        int otherId = (m.getSenderId() == userId) ? m.getReceiverId() : m.getSenderId();
        User otherUser = userRepository->findById(otherId);
        std::string otherUsername = (otherUser.getId() != -1) ? otherUser.getUsername() : "Unknown";
        
        ss << otherId << ";" 
           << otherUsername << ";" 
           << m.getContent() << ";" 
           << m.getTimestamp() << "|";
    }
    
    protocol::Message response(protocol::MsgCode::RECENT_CHATS_SUCCESS, ss.str());
    std::vector<uint8_t> data = response.serialize();
    send(clientFd, data.data(), data.size(), 0);
}

} // namespace server
