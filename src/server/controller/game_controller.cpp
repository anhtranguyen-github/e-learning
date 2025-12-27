#include "server/controller/game_controller.h"
#include "common/payloads.h"
#include "common/logger.h"
#include <vector>
#include <sstream>
#include <json/json.h>
#include "common/utils.h"
#include <unordered_map>
#include <unordered_set>

namespace server {

namespace {
std::string normalizeWhitespace(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    bool inSpace = false;
    for (char ch : value) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            if (!out.empty() && !inSpace) {
                out.push_back(' ');
            }
            inSpace = true;
        } else {
            out.push_back(ch);
            inSpace = false;
        }
    }
    if (!out.empty() && out.back() == ' ') {
        out.pop_back();
    }
    return out;
}

bool parseJsonArray(const std::string& raw, Json::Value& out) {
    Json::Reader reader;
    if (!reader.parse(raw, out)) {
        return false;
    }
    if (out.isArray()) {
        return true;
    }
    if (out.isObject() && out.isMember("questions") && out["questions"].isArray()) {
        out = out["questions"];
        return true;
    }
    return false;
}

bool computeGameScore(const Game& game, const std::string& detailsJson, double& score, std::string& feedback) {
    Json::Value questions;
    if (!parseJsonArray(game.getQuestionJson(), questions) || questions.empty()) {
        feedback = "Game data not found";
        return false;
    }

    Json::Value answers;
    if (!parseJsonArray(detailsJson, answers) || !answers.isArray()) {
        feedback = "Invalid submission data";
        return false;
    }

    const std::string& type = game.getType();
    int correctCount = 0;
    int total = static_cast<int>(questions.size());

    if (type == "sentence_match") {
        std::unordered_set<std::string> correctSentences;
        correctSentences.reserve(questions.size());
        for (const auto& q : questions) {
            std::string sentence = q.get("correct_sentence", "").asString();
            if (!sentence.empty()) {
                correctSentences.insert(normalizeWhitespace(sentence));
            }
        }

        for (const auto& ans : answers) {
            if (!ans.isObject()) {
                continue;
            }
            std::string expected = normalizeWhitespace(ans.get("question", "").asString());
            std::string response = normalizeWhitespace(ans.get("userResponse", "").asString());
            if (!expected.empty() && response == expected && correctSentences.count(expected) > 0) {
                correctCount++;
            }
        }
    } else if (type == "word_match") {
        std::unordered_map<std::string, std::string> pairs;
        pairs.reserve(questions.size());
        for (const auto& q : questions) {
            if (q.isMember("word_pair") && q["word_pair"].isArray() && q["word_pair"].size() >= 2) {
                std::string word = q["word_pair"][0].asString();
                std::string meaning = q["word_pair"][1].asString();
                if (!word.empty()) {
                    pairs[word] = meaning;
                }
            }
        }

        for (const auto& ans : answers) {
            if (!ans.isObject()) {
                continue;
            }
            std::string word = ans.get("word", "").asString();
            std::string response = ans.get("userResponse", "").asString();
            auto it = pairs.find(word);
            if (it != pairs.end() && response == it->second) {
                correctCount++;
            }
        }
    } else if (type == "image_match") {
        std::unordered_set<std::string> words;
        words.reserve(questions.size());
        for (const auto& q : questions) {
            std::string word = q.get("word", "").asString();
            if (!word.empty()) {
                words.insert(word);
            }
        }

        for (const auto& ans : answers) {
            if (!ans.isObject()) {
                continue;
            }
            std::string expected = ans.get("image", "").asString();
            std::string response = ans.get("userResponse", "").asString();
            if (!expected.empty() && words.count(expected) > 0 && response == expected) {
                correctCount++;
            }
        }
    } else {
        feedback = "Unsupported game type";
        return false;
    }

    score = static_cast<double>(correctCount) * 10.0;
    feedback = "You got " + std::to_string(correctCount) + " out of " + std::to_string(total) + " correct.";
    return true;
}
} // namespace

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
    if (userId <= 0) {
        protocol::Message response(protocol::MsgCode::GAME_SUBMIT_FAILURE, "Invalid session");
        sendMessage(clientFd, response);
        return;
    }

    int gameId = 0;
    try {
        gameId = std::stoi(req.gameId);
    } catch (...) {
        protocol::Message response(protocol::MsgCode::GAME_SUBMIT_FAILURE, "Invalid game ID");
        sendMessage(clientFd, response);
        return;
    }

    Game game = gameRepository->getGameById(gameId);
    if (game.getId() == 0) {
        protocol::Message response(protocol::MsgCode::GAME_SUBMIT_FAILURE, "Game not found");
        sendMessage(clientFd, response);
        return;
    }

    std::string targetType;
    const std::string& gameType = game.getType();
    if (gameType == "word_match") {
        targetType = "word_match_game";
    } else if (gameType == "sentence_match") {
        targetType = "sentence_match_game";
    } else if (gameType == "image_match") {
        targetType = "image_match_game";
    } else {
        protocol::Message response(protocol::MsgCode::GAME_SUBMIT_FAILURE, "Unsupported game type");
        sendMessage(clientFd, response);
        return;
    }

    double score = 0.0;
    std::string feedback;
    if (!req.detailsJson.empty()) {
        if (!computeGameScore(game, req.detailsJson, score, feedback)) {
            protocol::Message response(protocol::MsgCode::GAME_SUBMIT_FAILURE, feedback);
            sendMessage(clientFd, response);
            return;
        }
    } else {
        try {
            score = std::stod(req.score);
        } catch (...) {
            protocol::Message response(protocol::MsgCode::GAME_SUBMIT_FAILURE, "Invalid score");
            sendMessage(clientFd, response);
            return;
        }
        feedback = "Score submitted.";
    }

    if (!resultRepository || !resultRepository->saveResult(userId, targetType, gameId, score, "", feedback, "graded")) {
        protocol::Message response(protocol::MsgCode::GAME_SUBMIT_FAILURE, "Failed to save result");
        sendMessage(clientFd, response);
        return;
    }

    Payloads::GenericResponse resp;
    resp.success = true;
    resp.message = feedback + " Score: " + std::to_string(score);
    protocol::Message response(protocol::MsgCode::GAME_SUBMIT_SUCCESS, resp.serialize());
    sendMessage(clientFd, response);
}

} // namespace server
