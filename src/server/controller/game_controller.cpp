#include "server/controller/game_controller.h"
#include "common/payloads.h"
#include "common/logger.h"
#include <vector>
#include <sstream>
#include <json/json.h>
#include "common/utils.h"

namespace server {

GameController::GameController(std::shared_ptr<SessionManager> sm, 
                               std::shared_ptr<GameRepository> gr,
                               std::shared_ptr<ResultRepository> rr)
    : sessionManager(sm), gameRepository(gr), resultRepository(rr) {}

bool GameController::sendMessage(int clientFd, const protocol::Message& msg) {
    if (clientFd < 0) return false;
    std::vector<uint8_t> data = msg.serialize();
    return send(clientFd, data.data(), data.size(), 0) != -1;
}

void GameController::handleGameListRequest(int clientFd, const protocol::Message& msg) {
    auto payloadStr = msg.toString();
    Payloads::GameListRequest req;
    req.deserialize(payloadStr);

    if (!sessionManager->is_session_valid(req.sessionToken)) {
         // Should send failure or disconnect
         return; 
    }

    std::vector<std::string> types = gameRepository->getGameTypes();
    std::vector<std::string> serializedGames;

    for (const auto& type : types) {
        Payloads::GameMetadataDTO dto;
        dto.type = type;
        
          // Mock description based on type
        // if (type == "sentence_match") dto.description = "Match sentence fragments to form correct sentences.";
        // else if (type == "word_match") dto.description = "Match words to their meanings.";
        // else if (type == "image_match") dto.description = "Match words to the correct images.";
        // else dto.description = "A fun game to learn English.";

        serializedGames.push_back(dto.serialize());
    }

    std::string responsePayload = utils::join(serializedGames, ';'); // Use ; to separate items
    protocol::Message response(protocol::MsgCode::GAME_LIST_SUCCESS, responsePayload);
    sendMessage(clientFd, response);
}

void GameController::handleGameLevelListRequest(int clientFd, const protocol::Message& msg) {
    auto payloadStr = msg.toString();
    Payloads::GameLevelListRequest req;
    req.deserialize(payloadStr);

    if (!sessionManager->is_session_valid(req.sessionToken)) {
         return;
    }

    std::vector<Game> games = gameRepository->getLevelsByType(req.gameType);
    std::vector<std::string> serializedLevels;

    for (const auto& game : games) {
        Payloads::GameLevelDTO dto;
        dto.id = std::to_string(game.getId());
        dto.level = game.getLevel();
        dto.status = "unlocked"; // Logic to determine lock status could go here (e.g., check previous level result)
        serializedLevels.push_back(dto.serialize());
    }

    std::string responsePayload = utils::join(serializedLevels, ';');
    protocol::Message response(protocol::MsgCode::GAME_LEVEL_LIST_SUCCESS, responsePayload);
    sendMessage(clientFd, response);
}

void GameController::handleGameDataRequest(int clientFd, const protocol::Message& msg) {
    auto payloadStr = msg.toString();
    Payloads::GameDataRequest req;
    req.deserialize(payloadStr);

    if (!sessionManager->is_session_valid(req.sessionToken)) {
         return;
    }

    int gameId = std::stoi(req.gameId);
    Game game = gameRepository->getGameById(gameId);

    if (game.getId() == 0) {
        protocol::Message response(protocol::MsgCode::GAME_DATA_FAILURE, "Game not found");
        sendMessage(clientFd, response);
        return;
    }

    // Return Game Data
    Payloads::GameDataDTO dto = game.toDTO();

    // If this is an image_match game, we need to embed the actual image data
    if (game.getType() == "image_match") {
         Json::Value root;
         Json::Reader reader;
         if (reader.parse(dto.questionJson, root) && root.isArray()) {
             for (auto& item : root) {
                 if (item.isMember("image_url")) {
                     std::string imagePath = "data/images/" + item["image_url"].asString();
                     std::vector<char> imageData = utils::readFile(imagePath);
                     
                     if (!imageData.empty()) {
                         std::string base64Image = utils::base64Encode(imageData);
                         // Replace the URL with the Data URI scheme
                         item["image_url"] = "data:image/png;base64," + base64Image;
                     } else {
                         if (logger::serverLogger) {
                             logger::serverLogger->warn("Failed to read image file: " + imagePath);
                         }
                     }
                 }
             }
             Json::FastWriter writer;
             dto.questionJson = writer.write(root);
         }
    }

    protocol::Message response(protocol::MsgCode::GAME_DATA_SUCCESS, dto.serialize());
    sendMessage(clientFd, response);
}

void GameController::handleGameSubmitRequest(int clientFd, const protocol::Message& msg) {
    auto payloadStr = msg.toString();
    Payloads::GameSubmitRequest req;
    req.deserialize(payloadStr);

    if (!sessionManager->is_session_valid(req.sessionToken)) {
        protocol::Message response(protocol::MsgCode::GAME_SUBMIT_FAILURE, "Invalid session");
        sendMessage(clientFd, response);
        return;
    }

    int userId = sessionManager->get_user_id_by_session(req.sessionToken);
    int gameId = std::stoi(req.gameId);
    double score = std::stod(req.score);

    // Save result
    bool success = resultRepository->saveResult(userId, "game", gameId, score, req.detailsJson, "Game Completed", "graded");

    if (success) {
        Payloads::GenericResponse resp;
        resp.success = true;
        resp.message = "Game result saved successfully";
        protocol::Message response(protocol::MsgCode::GAME_SUBMIT_SUCCESS, resp.serialize());
        sendMessage(clientFd, response);
    } else {
         protocol::Message response(protocol::MsgCode::GAME_SUBMIT_FAILURE, "Failed to save result");
         sendMessage(clientFd, response);
    }
}

} // namespace server
