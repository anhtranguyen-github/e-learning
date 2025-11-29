#include "server/repository/chat_repository.h"
#include "common/logger.h"
#include <iostream>

namespace server {

ChatRepository::ChatRepository(std::shared_ptr<Database> db) : db(db) {}

int ChatRepository::saveMessage(const ChatMessage& message) {
    std::string sql = "INSERT INTO chat_messages (sender_id, receiver_id, content, message_type, created_at, is_read) "
                      "VALUES ($1, $2, $3, $4, NOW(), $5) RETURNING id";
    
    std::vector<std::string> params = {
        std::to_string(message.getSenderId()),
        std::to_string(message.getReceiverId()),
        message.getContent(),
        message.getMessageType(),
        message.getIsRead() ? "true" : "false"
    };

    if (logger::serverLogger) {
        logger::serverLogger->debug("Saving message: sender=" + params[0] + ", receiver=" + params[1]);
    }

    std::vector<const char*> paramValues;
    for (const auto& p : params) {
        paramValues.push_back(p.c_str());
    }

    try {
        PGresult* res = db->execParams(sql, params.size(), paramValues.data());
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            logger::serverLogger->error("Failed to save message: " + std::string(PQerrorMessage(db->getConnection())));
            PQclear(res);
            return -1;
        }

        int id = std::stoi(PQgetvalue(res, 0, 0));
        PQclear(res);
        return id;
    } catch (const std::exception& e) {
        logger::serverLogger->error("Exception in saveMessage: " + std::string(e.what()));
        return -1;
    }
}

std::vector<ChatMessage> ChatRepository::getChatHistory(int userId1, int userId2, int limit, int offset) {
    std::vector<ChatMessage> messages;
    
    // Get messages where (sender=u1 AND receiver=u2) OR (sender=u2 AND receiver=u1)
    // Order by created_at ASC (oldest first)
    std::string sql = "SELECT id, sender_id, receiver_id, content, message_type, created_at, is_read "
                      "FROM chat_messages "
                      "WHERE (sender_id = $1 AND receiver_id = $2) OR (sender_id = $2 AND receiver_id = $1) "
                      "ORDER BY created_at ASC "
                      "LIMIT $3 OFFSET $4";

    std::vector<std::string> params = {
        std::to_string(userId1),
        std::to_string(userId2),
        std::to_string(limit),
        std::to_string(offset)
    };

    std::vector<const char*> paramValues;
    for (const auto& p : params) {
        paramValues.push_back(p.c_str());
    }

    try {
        PGresult* res = db->execParams(sql, params.size(), paramValues.data());
        
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            logger::serverLogger->error("Failed to get chat history: " + std::string(PQerrorMessage(db->getConnection())));
            PQclear(res);
            return messages;
        }

        int rows = PQntuples(res);
        for (int i = 0; i < rows; ++i) {
            int id = std::stoi(PQgetvalue(res, i, 0));
            int senderId = std::stoi(PQgetvalue(res, i, 1));
            int receiverId = std::stoi(PQgetvalue(res, i, 2));
            std::string content = PQgetvalue(res, i, 3);
            std::string messageType = PQgetvalue(res, i, 4);
            std::string timestamp = PQgetvalue(res, i, 5);
            bool isRead = std::string(PQgetvalue(res, i, 6)) == "t";

            messages.emplace_back(id, senderId, receiverId, content, messageType, timestamp, isRead);
        }
        
        PQclear(res);
    } catch (const std::exception& e) {
        logger::serverLogger->error("Exception in getChatHistory: " + std::string(e.what()));
    }

    return messages;
}

std::vector<ChatMessage> ChatRepository::getRecentChats(int userId) {
    std::vector<ChatMessage> messages;

    // Complex query to get the latest message for each conversation partner
    // We want to find all unique users that 'userId' has chatted with, and get the most recent message for each.
    // 
    // Strategy:
    // 1. Find all messages where userId is sender or receiver.
    // 2. Determine the "other" user ID.
    // 3. Group by "other" user ID and find MAX(created_at).
    // 4. Join back to get the message details.
    
    std::string sql = 
        "WITH LastMessages AS ( "
        "    SELECT "
        "        CASE WHEN sender_id = $1 THEN receiver_id ELSE sender_id END AS other_user_id, "
        "        MAX(created_at) as max_time "
        "    FROM chat_messages "
        "    WHERE sender_id = $1 OR receiver_id = $1 "
        "    GROUP BY other_user_id "
        ") "
        "SELECT cm.id, cm.sender_id, cm.receiver_id, cm.content, cm.message_type, cm.created_at, cm.is_read "
        "FROM chat_messages cm "
        "JOIN LastMessages lm ON "
        "    (cm.created_at = lm.max_time) AND "
        "    ((cm.sender_id = $1 AND cm.receiver_id = lm.other_user_id) OR "
        "     (cm.sender_id = lm.other_user_id AND cm.receiver_id = $1)) "
        "ORDER BY cm.created_at DESC";

    std::vector<std::string> params = { std::to_string(userId) };
    std::vector<const char*> paramValues = { params[0].c_str() };

    try {
        PGresult* res = db->execParams(sql, params.size(), paramValues.data());
        
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            logger::serverLogger->error("Failed to get recent chats: " + std::string(PQerrorMessage(db->getConnection())));
            PQclear(res);
            return messages;
        }

        int rows = PQntuples(res);
        for (int i = 0; i < rows; ++i) {
            int id = std::stoi(PQgetvalue(res, i, 0));
            int senderId = std::stoi(PQgetvalue(res, i, 1));
            int receiverId = std::stoi(PQgetvalue(res, i, 2));
            std::string content = PQgetvalue(res, i, 3);
            std::string messageType = PQgetvalue(res, i, 4);
            std::string timestamp = PQgetvalue(res, i, 5);
            bool isRead = std::string(PQgetvalue(res, i, 6)) == "t";

            messages.emplace_back(id, senderId, receiverId, content, messageType, timestamp, isRead);
        }
        
        PQclear(res);
    } catch (const std::exception& e) {
        logger::serverLogger->error("Exception in getRecentChats: " + std::string(e.what()));
    }

    return messages;
}

void ChatRepository::markMessagesAsRead(int senderId, int receiverId) {
    std::string sql = "UPDATE chat_messages SET is_read = TRUE "
                      "WHERE sender_id = $1 AND receiver_id = $2 AND is_read = FALSE";
    
    std::vector<std::string> params = {
        std::to_string(senderId),
        std::to_string(receiverId)
    };

    std::vector<const char*> paramValues;
    for (const auto& p : params) {
        paramValues.push_back(p.c_str());
    }

    try {
        PGresult* res = db->execParams(sql, params.size(), paramValues.data());
        PQclear(res);
    } catch (const std::exception& e) {
        logger::serverLogger->error("Exception in markMessagesAsRead: " + std::string(e.what()));
    }
}

} // namespace server
