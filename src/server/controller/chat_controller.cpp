#include "server/controller/chat_controller.h"
#include "server/client_handler.h"
#include "common/logger.h"
#include "common/payloads.h"
#include "common/utils.h"
#include <sys/socket.h>
#include <sstream>
#include <vector>

namespace server {

ChatController::ChatController(std::shared_ptr<SessionManager> sessionMgr, 
                           std::shared_ptr<ConnectionManager> connMgr,
                           std::shared_ptr<Database> database)
    : sessionManager(sessionMgr), connectionManager(connMgr), db(database) {
}

void ChatController::sendMessage(int clientFd, const protocol::Message& msg) {
    std::vector<uint8_t> data = msg.serialize();
    send(clientFd, data.data(), data.size(), 0);
}

void ChatController::handle_private_message(int clientFd, const protocol::Message& msg) {
    std::string payload = msg.toString();
    Payloads::PrivateMessageRequest req;
    req.deserialize(payload);

    std::string sessionToken = req.sessionToken;
    std::string recipient_username = req.recipient;
    std::string message_content = req.message;

    if (recipient_username.empty() || message_content.empty()) {
         Payloads::GenericResponse resp;
         resp.success = false;
         resp.message = "Invalid message format";
         protocol::Message error_msg(protocol::MsgCode::CHAT_MESSAGE_FAILURE, resp.serialize());
         sendMessage(clientFd, error_msg);
         return;
    }

    int sender_id = sessionManager->get_user_id_by_session(sessionToken);
    if (sender_id == -1) {
        Payloads::GenericResponse resp;
        resp.success = false;
        resp.message = "Invalid session";
        protocol::Message error_msg(protocol::MsgCode::CHAT_MESSAGE_FAILURE, resp.serialize());
        sendMessage(clientFd, error_msg);
        return;
    }

    sessionManager->update_session(sessionToken);

    // Get recipient ID directly from DB since ConnectionManager doesn't handle auth/users anymore
    int recipient_id = -1;
    std::string query = "SELECT user_id FROM users WHERE username = $1";
    const char* values[] = {recipient_username.c_str()};
    PGresult* res = db->execParams(query, 1, values);
    
    if (res && PQntuples(res) == 1) {
        recipient_id = std::stoi(PQgetvalue(res, 0, 0));
    }
    if (res) PQclear(res);

    if (recipient_id == -1) {
        Payloads::GenericResponse resp;
        resp.success = false;
        resp.message = "Recipient not found";
        protocol::Message error_msg(protocol::MsgCode::CHAT_MESSAGE_FAILURE, resp.serialize());
        sendMessage(clientFd, error_msg);
        return;
    }

    // Forward message if recipient is online
    auto recipient_client = connectionManager->get_client(recipient_id);
    if (recipient_client) {
        protocol::Message forward_msg(protocol::MsgCode::CHAT_PRIVATE_RECEIVE, std::to_string(sender_id) + ":" + message_content);
        recipient_client->send_message(forward_msg);
        
        Payloads::GenericResponse resp;
        resp.success = true;
        resp.message = "Message sent";
        protocol::Message success_msg(protocol::MsgCode::CHAT_MESSAGE_SUCCESS, resp.serialize());
        sendMessage(clientFd, success_msg);
    } else {
        Payloads::GenericResponse resp;
        resp.success = false;
        resp.message = "Recipient is offline";
        protocol::Message error_msg(protocol::MsgCode::CHAT_MESSAGE_FAILURE, resp.serialize());
        sendMessage(clientFd, error_msg);
    }
}

void ChatController::handle_chat_history(int clientFd, const protocol::Message& /*msg*/) {
    Payloads::GenericResponse resp;
    resp.success = false;
    resp.message = "Chat history is disabled";
    protocol::Message error_msg(protocol::MsgCode::CHAT_HISTORY_FAILURE, resp.serialize());
    sendMessage(clientFd, error_msg);
}

} // namespace server
