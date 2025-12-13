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

void ChatController::handleCallInitiate(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    Payloads::VoiceCallRequest req;
    req.deserialize(payload);

    int callerId = sessionManager->get_user_id_by_session(req.sessionToken);
    if (callerId == -1) {
        protocol::Message response(protocol::MsgCode::CALL_FAILED, "Invalid session");
        std::vector<uint8_t> data = response.serialize();
        send(clientFd, data.data(), data.size(), 0);
        return;
    }

    User caller = userRepository->findById(callerId);
    int targetId = userRepository->getUserId(req.targetUser);
    
    if (targetId == -1) {
        protocol::Message response(protocol::MsgCode::CALL_FAILED, "User not found");
        std::vector<uint8_t> data = response.serialize();
        send(clientFd, data.data(), data.size(), 0);
        return;
    }

    // Check if target is online
    bool isTargetOnline = connectionManager->isUserOnline(targetId);
    
    // Check if sender is already in a call (Single Call Session)
    if (activeUsersInCall.count(caller.getUsername()) > 0 || pendingCalls.count(caller.getUsername()) > 0) {
        protocol::Message response(protocol::MsgCode::CALL_FAILED, "You are already in a call");
        std::vector<uint8_t> data = response.serialize();
        send(clientFd, data.data(), data.size(), 0);
        return;
    }

    // Check if target is busy (in a call or receiving a call)
    // Note: checking pendingCalls by receiver key (targetUser). Also check if target is a caller in pendingCalls?
    bool targetIsBusy = false;
    if (activeUsersInCall.count(req.targetUser) > 0) targetIsBusy = true;
    if (pendingCalls.count(req.targetUser) > 0) targetIsBusy = true; // Receiving a call
    
    // Check if target is initiating a call (caller in pendingCalls)
    for (const auto& pair : pendingCalls) {
        if (pair.second.caller == req.targetUser) {
            targetIsBusy = true;
            break;
        }
    }

    if (targetIsBusy) {
        protocol::Message response(protocol::MsgCode::CALL_FAILED, "User is busy");
        std::vector<uint8_t> data = response.serialize();
        send(clientFd, data.data(), data.size(), 0);
        
        // Log busy attempt
        ChatMessage sysMsg;
        sysMsg.setSenderId(callerId);
        sysMsg.setReceiverId(targetId);
        sysMsg.setContent("Call Failed: User Busy");
        sysMsg.setMessageType("SYSTEM");
        sysMsg.setIsRead(false);
        chatRepository->saveMessage(sysMsg);
        return;
    }

    // Register Pending Call (Wait State)
    PendingCall call;
    call.caller = caller.getUsername();
    call.receiver = req.targetUser;
    call.startTime = std::chrono::steady_clock::now();
    pendingCalls[req.targetUser] = call;

    // Send incoming call notification ONLY if target is online
    if (isTargetOnline) {
        Payloads::VoiceCallNotification notification;
        notification.callerUsername = caller.getUsername();
        notification.callerId = std::to_string(callerId);
        
        protocol::Message callNotification(protocol::MsgCode::CALL_INCOMING, notification.serialize());
        connectionManager->sendToUser(targetId, callNotification);
    } 
    // If offline, we effectively simulate ringing by doing nothing here and waiting for timeout.

    if (logger::serverLogger) {
        logger::serverLogger->info("Call initiated from " + caller.getUsername() + " to " + req.targetUser);
    }


    if (logger::serverLogger) {
        logger::serverLogger->info("Call initiated from " + caller.getUsername() + " to " + req.targetUser);
    }

    // Save SYSTEM message
    ChatMessage sysMsg;
    sysMsg.setSenderId(callerId);
    sysMsg.setReceiverId(targetId);
    sysMsg.setContent("Call Initiated");
    sysMsg.setMessageType("SYSTEM");
    sysMsg.setIsRead(false);
    chatRepository->saveMessage(sysMsg);
}

void ChatController::handleCallAnswer(int clientFd, const protocol::Message& msg) {
    (void)clientFd;
    std::string payload = msg.toString();
    Payloads::VoiceCallRequest req;
    req.deserialize(payload);

    int answererId = sessionManager->get_user_id_by_session(req.sessionToken);
    if (answererId == -1) return;

    User answerer = userRepository->findById(answererId);
    int callerId = userRepository->getUserId(req.targetUser);
    if (callerId == -1) return;

    // Verify this call was pending
    std::string answererName = answerer.getUsername();
    if (pendingCalls.count(answererName) == 0 || pendingCalls[answererName].caller != req.targetUser) {
        // Call not found or expired
        return; 
    }

    // Remove from pending
    pendingCalls.erase(answererName);

    // Add to active users
    activeUsersInCall.insert(answererName);
    activeUsersInCall.insert(req.targetUser);

    // Notify caller that call was answered
    Payloads::VoiceCallNotification notification;
    notification.callerUsername = answerer.getUsername();
    notification.callerId = std::to_string(answererId);
    
    protocol::Message response(protocol::MsgCode::CALL_ANSWER_REQUEST, notification.serialize());
    connectionManager->sendToUser(callerId, response);

    if (logger::serverLogger) {
        logger::serverLogger->info("Call answered by " + answerer.getUsername());
    }

    // Save SYSTEM message
    ChatMessage sysMsg;
    sysMsg.setSenderId(answererId);
    sysMsg.setReceiverId(callerId);
    sysMsg.setContent("Call Answered");
    sysMsg.setMessageType("SYSTEM");
    sysMsg.setIsRead(false);
    chatRepository->saveMessage(sysMsg);
}

void ChatController::handleCallDecline(int clientFd, const protocol::Message& msg) {
    (void)clientFd;
    std::string payload = msg.toString();
    Payloads::VoiceCallRequest req;
    req.deserialize(payload);

    int declinerId = sessionManager->get_user_id_by_session(req.sessionToken);
    if (declinerId == -1) return;

    User decliner = userRepository->findById(declinerId);
    int callerId = userRepository->getUserId(req.targetUser);
    if (callerId == -1) return;

    std::string declinerName = decliner.getUsername();

    // Remove from pending if exists
    if (pendingCalls.count(declinerName) > 0) {
        pendingCalls.erase(declinerName);
    }

    // Notify caller that call was declined
    protocol::Message response(protocol::MsgCode::CALL_ENDED, "Call declined by " + decliner.getUsername());
    connectionManager->sendToUser(callerId, response);

    if (logger::serverLogger) {
        logger::serverLogger->info("Call declined by " + decliner.getUsername());
    }

    // Save SYSTEM message
    ChatMessage sysMsg;
    sysMsg.setSenderId(declinerId);
    sysMsg.setReceiverId(callerId);
    sysMsg.setContent("Call Declined");
    sysMsg.setMessageType("SYSTEM");
    sysMsg.setIsRead(false);
    chatRepository->saveMessage(sysMsg);
}

void ChatController::handleCallEnd(int clientFd, const protocol::Message& msg) {
    (void)clientFd;
    std::string payload = msg.toString();
    Payloads::VoiceCallRequest req;
    req.deserialize(payload);

    int enderId = sessionManager->get_user_id_by_session(req.sessionToken);
    if (enderId == -1) return;

    User ender = userRepository->findById(enderId);
    int otherId = userRepository->getUserId(req.targetUser);
    if (otherId == -1) return;

    // Remove both from active users
    activeUsersInCall.erase(ender.getUsername());
    activeUsersInCall.erase(req.targetUser);

    // Also check pending calls just in case (e.g. cancelling an initiated call)
    // If Ender was the Caller (cancelling request)
    // We check if 'req.targetUser' has a pending call from 'ender'
    if (pendingCalls.count(req.targetUser) > 0 && pendingCalls[req.targetUser].caller == ender.getUsername()) {
        pendingCalls.erase(req.targetUser);
    }
    // If Ender was the Receiver (should match handleCallDecline logic, but maybe they accepted then ended?)
    // If they were in pending, handleCallEnd (cancel) is valid too.
    if (pendingCalls.count(ender.getUsername()) > 0) {
        pendingCalls.erase(ender.getUsername());
    }

    // Notify other party
    protocol::Message response(protocol::MsgCode::CALL_ENDED, "Call ended by " + ender.getUsername());
    connectionManager->sendToUser(otherId, response);

    if (logger::serverLogger) {
        logger::serverLogger->info("Call ended by " + ender.getUsername());
    }

    // Save SYSTEM message
    ChatMessage sysMsg;
    sysMsg.setSenderId(enderId);
    sysMsg.setReceiverId(otherId);
    sysMsg.setContent("Call Ended");
    sysMsg.setMessageType("SYSTEM");
    sysMsg.setIsRead(false);
    chatRepository->saveMessage(sysMsg);
}

void ChatController::processCallTimeouts() {
    auto now = std::chrono::steady_clock::now();
    std::vector<std::string> timedOutReceivers;

    // Check for timeouts
    for (const auto& pair : pendingCalls) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - pair.second.startTime).count();
        if (elapsed > 7) { // 5s requested + 2s buffer
            timedOutReceivers.push_back(pair.first);
        }
    }

    // Handle timeouts
    for (const auto& receiver : timedOutReceivers) {
        const PendingCall& call = pendingCalls[receiver];
        
        int callerId = userRepository->getUserId(call.caller);
        int receiverId = userRepository->getUserId(call.receiver);

        // Notify Caller: User busy (Unified message for offline/busy/timeout)
        if (callerId != -1) {
            protocol::Message response(protocol::MsgCode::CALL_FAILED, "User is busy");
            connectionManager->sendToUser(callerId, response);
            
            // Log for Caller
            ChatMessage sysMsg;
            sysMsg.setSenderId(callerId); // Caller is "Sender" of this event? Or Self?
            // Usually SYSTEM msg has system id? But here we emulate.
            // Let's use callerId -> receiverId for "Call Failed"
            sysMsg.setSenderId(callerId);
            sysMsg.setReceiverId(receiverId != -1 ? receiverId : 0); // Need receiver ID
            sysMsg.setContent("Call Failed: User is busy");
            sysMsg.setMessageType("SYSTEM");
            sysMsg.setIsRead(true); // Read by caller implicitly?
            chatRepository->saveMessage(sysMsg);
        }

        // Notify Receiver: Cancel/Missed Call (to stop ringing if online, and log history)
        if (receiverId != -1) {
            protocol::Message response(protocol::MsgCode::CALL_ENDED, "Missed call");
            connectionManager->sendToUser(receiverId, response);
            
            // Log Missed Call for Receiver
            ChatMessage sysMsg;
            sysMsg.setSenderId(callerId != -1 ? callerId : 0);
            sysMsg.setReceiverId(receiverId);
            sysMsg.setContent("Missed Call");
            sysMsg.setMessageType("SYSTEM");
            sysMsg.setIsRead(false);
            chatRepository->saveMessage(sysMsg);
        }

        if (logger::serverLogger) {
            logger::serverLogger->info("Call timed out: " + call.caller + " -> " + call.receiver);
        }
        
        // Remove from pending
        pendingCalls.erase(receiver);
    }
}

} // namespace server
