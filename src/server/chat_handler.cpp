#include "server/chat_handler.h"
#include "common/payloads.h"
#include "server/client_handler.h"
#include "server/user_manager.h"
#include "server/database.h"
#include "common/logger.h"

namespace server {

ChatHandler::ChatHandler(UserManager& user_manager, Database& db)
    : user_manager_(user_manager), db_(db) {}

void ChatHandler::handle_chat_message(ClientHandler* client, const protocol::Message& msg) {
    switch (msg.code) {
        case protocol::MsgCode::SEND_CHAT_PRIVATE_REQUEST:
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
    Payloads::PrivateMessageRequest req;
    req.deserialize(payload);

    std::string sessionToken = req.sessionToken;
    std::string recipient_username = req.recipient;
    std::string message_content = req.message;

    // Validate session (ChatHandler doesn't seem to have SessionManager directly injected in constructor? 
    // Wait, it takes UserManager and Database. It seems ChatHandler assumes the client is already authenticated/identified by ClientHandler?
    // The original code parsed sessionToken but didn't seem to use it for validation, it used client->get_user_id().
    // Let's check the original code again.
    // Original: 
    // std::string recipient_username = parts[1];
    // std::string message_content = parts[2];
    // It seems the original code expected <token>;<recipient>;<message> but treated parts[0] as token?
    // Actually, looking at `handle_private_message`:
    // parts[1] is recipient, parts[2] is message. parts[0] is presumably token.
    // But it uses `client->get_user_id()` for sender_id.
    // So the token in payload might be redundant or used for double check?
    // The original code didn't validate token explicitly in this method.
    // I will stick to extracting the fields.
    
    if (recipient_username.empty() || message_content.empty()) {
         protocol::Message error_msg(protocol::MsgCode::CHAT_MESSAGE_FAILURE, "Invalid message format");
         client->send_message(error_msg);
         return;
    }

    int sender_id = client->get_user_id();
    int recipient_id = user_manager_.get_user_id(recipient_username);

    if (recipient_id == -1) {
        protocol::Message error_msg(protocol::MsgCode::CHAT_MESSAGE_FAILURE, "Recipient not found");
        client->send_message(error_msg);
        return;
    }

    // Forward message if recipient is online
    ClientHandler* recipient_client = user_manager_.get_client(recipient_id);
    if (recipient_client) {
        protocol::Message forward_msg(protocol::MsgCode::CHAT_PRIVATE_RECEIVE, std::to_string(sender_id) + ":" + message_content);
        recipient_client->send_message(forward_msg);
        
        protocol::Message success_msg(protocol::MsgCode::CHAT_MESSAGE_SUCCESS, "Message sent");
        client->send_message(success_msg);
    } else {
        protocol::Message error_msg(protocol::MsgCode::CHAT_MESSAGE_FAILURE, "Recipient is offline");
        client->send_message(error_msg);
    }
}

void ChatHandler::handle_chat_history(ClientHandler* client, const protocol::Message& /*msg*/) {
    protocol::Message error_msg(protocol::MsgCode::CHAT_HISTORY_FAILURE, "Chat history is disabled");
    client->send_message(error_msg);
}

} // namespace server
