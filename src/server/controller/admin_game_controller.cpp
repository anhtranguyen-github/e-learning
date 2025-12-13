#include "server/controller/admin_game_controller.h"
#include "common/payloads.h"
#include "common/logger.h"
#include <vector>
#include <sstream>

namespace server {

AdminGameController::AdminGameController(std::shared_ptr<SessionManager> sm, 
                                         std::shared_ptr<GameRepository> gr)
    : sessionManager(sm), gameRepository(gr) {}

bool AdminGameController::sendMessage(int clientFd, const protocol::Message& msg) {
    if (clientFd < 0) return false;
    std::vector<uint8_t> data = msg.serialize();
    return send(clientFd, data.data(), data.size(), 0) != -1;
}

void AdminGameController::handleGameCreateRequest(int clientFd, const protocol::Message& msg) {
    auto payloadStr = msg.toString();
    Payloads::GameCreateRequest req;
    req.deserialize(payloadStr);

    // Note: RBAC Middleware should have already verified the admin role.
    // We double check session validity here just in case.
    if (!sessionManager->is_session_valid(req.sessionToken)) {
         protocol::Message response(protocol::MsgCode::GAME_CREATE_FAILURE, "Invalid session");
         sendMessage(clientFd, response);
         return; 
    }

    // Role check (redundant if middleware works, but safe)
    if (sessionManager->get_user_role_by_fd(clientFd) != "admin") {
         protocol::Message response(protocol::MsgCode::GAME_CREATE_FAILURE, "Unauthorized");
         sendMessage(clientFd, response);
         return;
    }

    Game newGame(0, req.type, req.level, req.questionJson);
    int newId = gameRepository->createGame(newGame);

    if (newId > 0) {
        Payloads::GenericResponse resp;
        resp.success = true;
        resp.message = std::to_string(newId); // Return the ID of created game
        protocol::Message response(protocol::MsgCode::GAME_CREATE_SUCCESS, resp.serialize());
        sendMessage(clientFd, response);
    } else {
        protocol::Message response(protocol::MsgCode::GAME_CREATE_FAILURE, "Failed to create game");
        sendMessage(clientFd, response);
    }
}

void AdminGameController::handleGameUpdateRequest(int clientFd, const protocol::Message& msg) {
    auto payloadStr = msg.toString();
    Payloads::GameUpdateRequest req;
    req.deserialize(payloadStr);

    if (!sessionManager->is_session_valid(req.sessionToken)) {
         protocol::Message response(protocol::MsgCode::GAME_UPDATE_FAILURE, "Invalid session");
         sendMessage(clientFd, response);
         return; 
    }

    if (sessionManager->get_user_role_by_fd(clientFd) != "admin") {
         protocol::Message response(protocol::MsgCode::GAME_UPDATE_FAILURE, "Unauthorized");
         sendMessage(clientFd, response);
         return;
    }

    Game game(std::stoi(req.gameId), req.type, req.level, req.questionJson);
    bool success = gameRepository->updateGame(game);

    if (success) {
        Payloads::GenericResponse resp;
        resp.success = true;
        resp.message = "Game updated successfully";
        protocol::Message response(protocol::MsgCode::GAME_UPDATE_SUCCESS, resp.serialize());
        sendMessage(clientFd, response);
    } else {
        protocol::Message response(protocol::MsgCode::GAME_UPDATE_FAILURE, "Failed to update game");
        sendMessage(clientFd, response);
    }
}

void AdminGameController::handleGameDeleteRequest(int clientFd, const protocol::Message& msg) {
    auto payloadStr = msg.toString();
    Payloads::GameDeleteRequest req;
    req.deserialize(payloadStr);

    if (!sessionManager->is_session_valid(req.sessionToken)) {
         protocol::Message response(protocol::MsgCode::GAME_DELETE_FAILURE, "Invalid session");
         sendMessage(clientFd, response);
         return; 
    }

    if (sessionManager->get_user_role_by_fd(clientFd) != "admin") {
         protocol::Message response(protocol::MsgCode::GAME_DELETE_FAILURE, "Unauthorized");
         sendMessage(clientFd, response);
         return;
    }

    bool success = gameRepository->deleteGame(std::stoi(req.gameId));

    if (success) {
        Payloads::GenericResponse resp;
        resp.success = true;
        resp.message = "Game deleted successfully";
        protocol::Message response(protocol::MsgCode::GAME_DELETE_SUCCESS, resp.serialize());
        sendMessage(clientFd, response);
    } else {
        protocol::Message response(protocol::MsgCode::GAME_DELETE_FAILURE, "Failed to delete game");
        sendMessage(clientFd, response);
    }
}

} // namespace server
