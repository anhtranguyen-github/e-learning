#include "server/chat_handler.h"
#include "server/client_handler.h"
#include "server/user_manager.h"
#include "server/database.h"
#include "common/logger.h"

namespace server {

ChatHandler::ChatHandler(UserManager& user_manager, Database& db)
    : user_manager_(user_manager), db_(db) {}

void ChatHandler::handle_chat_message(ClientHandler* client, const protocol::Message& msg) {
    switch (msg.code) {
        case protocol::MsgCode::CHAT_PRIVATE_REQUEST:
            handle_private_message(client, msg);
            break;
        case protocol::MsgCode::CHAT_HISTORY_REQUEST:
            handle_chat_history(client, msg);
            break;
        default:
            std::string log_message = "Unknown chat message code: " + std::to_string(static_cast<int>(msg.code));
            logger::serverLogger->log(logger::LogLevel::WARN, log_message);
            // Optionally send an error back to the client
            protocol::Message error_msg(protocol::MsgCode::UNKNOWN_COMMAND_FAILURE, "Unknown chat command");
            client->send_message(error_msg);
            break;
    }
}

void ChatHandler::handle_private_message(ClientHandler* client, const protocol::Message& msg) {
    std::string payload = msg.toString();
    std::vector<std::string> parts;
    std::istringstream iss(payload);
    std::string part;
    while (std::getline(iss, part, ';')) {
        parts.push_back(part);
    }

    if (parts.size() < 3) {
        protocol::Message error_msg(protocol::MsgCode::CHAT_MESSAGE_FAILURE, "Invalid message format");
        client->send_message(error_msg);
        return;
    }

    std::string recipient_username = parts[1];
    std::string message_content = parts[2];

    int sender_id = client->get_user_id();
    int recipient_id = user_manager_.get_user_id(recipient_username);

    if (recipient_id == -1) {
        protocol::Message error_msg(protocol::MsgCode::CHAT_MESSAGE_FAILURE, "Recipient not found");
        client->send_message(error_msg);
        return;
    }

    // Store message in DB
    std::string query = "INSERT INTO chat_messages (sender_id, receiver_id, message) VALUES ($1, $2, $3)";
    const char* values[] = {std::to_string(sender_id).c_str(), std::to_string(recipient_id).c_str(), message_content.c_str()};
    db_.execParams(query, 3, values);

    // Forward message if recipient is online
    ClientHandler* recipient_client = user_manager_.get_client(recipient_id);
    if (recipient_client) {
        protocol::Message forward_msg(protocol::MsgCode::CHAT_PRIVATE_RECEIVE, std::to_string(sender_id) + ":" + message_content);
        recipient_client->send_message(forward_msg);
    } else {
        // Optionally, store the message as unread
        // For now, we just log it
        std::string log_message = "Recipient " + recipient_username + " is offline. Message stored.";
        logger::serverLogger->log(logger::LogLevel::INFO, log_message);
    }

    protocol::Message success_msg(protocol::MsgCode::CHAT_MESSAGE_SUCCESS, "Message sent");
    client->send_message(success_msg);
}

void ChatHandler::handle_chat_history(ClientHandler* client, const protocol::Message& msg) {
    std::string payload = msg.toString();
    std::vector<std::string> parts;
    std::istringstream iss(payload);
    std::string part;
    while (std::getline(iss, part, ';')) {
        parts.push_back(part);
    }

    if (parts.size() < 2) {
        protocol::Message error_msg(protocol::MsgCode::CHAT_HISTORY_FAILURE, "Invalid request format");
        client->send_message(error_msg);
        return;
    }

    std::string other_username = parts[1];
    int user1_id = client->get_user_id();
    int user2_id = user_manager_.get_user_id(other_username);

    if (user2_id == -1) {
        protocol::Message error_msg(protocol::MsgCode::CHAT_HISTORY_FAILURE, "User not found");
        client->send_message(error_msg);
        return;
    }

    std::string query = "SELECT sender_id, message, timestamp FROM chat_messages WHERE (sender_id = $1 AND receiver_id = $2) OR (sender_id = $2 AND receiver_id = $1) ORDER BY timestamp ASC";
    const char* values[] = {std::to_string(user1_id).c_str(), std::to_string(user2_id).c_str()};
    PGresult* res = db_.execParams(query, 2, values);

    if (!res) {
        protocol::Message error_msg(protocol::MsgCode::CHAT_HISTORY_FAILURE, "Failed to retrieve chat history");
        client->send_message(error_msg);
        return;
    }

    std::string history;
    for (int i = 0; i < PQntuples(res); ++i) {
        history += std::string(PQgetvalue(res, i, 0)) + ":" + std::string(PQgetvalue(res, i, 1)) + "\n";
    }

    PQclear(res);

    protocol::Message history_msg(protocol::MsgCode::CHAT_HISTORY_SUCCESS, history);
    client->send_message(history_msg);
}

} // namespace server
