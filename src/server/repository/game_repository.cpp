#include "server/repository/game_repository.h"
#include "common/logger.h"
#include <algorithm>

namespace server {

GameRepository::GameRepository(std::shared_ptr<Database> database) : db(database) {
}

bool GameRepository::parseGameFromRow(PGresult* result, int row, Game& game) const {
    try {
        int col_id = PQfnumber(result, "game_id");
        if (col_id != -1) {
            game.setId(std::stoi(PQgetvalue(result, row, col_id)));
        }

        int col_type = PQfnumber(result, "type");
        if (col_type != -1) {
            game.setType(PQgetvalue(result, row, col_type));
        }

        int col_level = PQfnumber(result, "level");
        if (col_level != -1) {
            game.setLevel(PQgetvalue(result, row, col_level));
        }

        int col_question = PQfnumber(result, "question");
        if (col_question != -1) {
            game.setQuestionJson(PQgetvalue(result, row, col_question));
        }

        return true;
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Error parsing game from row: " + std::string(e.what()));
        }
        return false;
    }
}

std::vector<std::string> GameRepository::getGameTypes() {
    std::vector<std::string> types;
    if (!db || !db->isConnected()) return types;

    std::string query = "SELECT DISTINCT type FROM game_items ORDER BY type";
    PGresult* result = db->query(query);

    if (!result) return types;

    int rowCount = PQntuples(result);
    for (int i = 0; i < rowCount; i++) {
        types.push_back(PQgetvalue(result, i, 0)); // 0 is the index of 'type'
    }
    PQclear(result);
    return types;
}

std::vector<Game> GameRepository::getLevelsByType(const std::string& type) {
    std::vector<Game> games;
    if (!db || !db->isConnected()) return games;

    std::string query = "SELECT * FROM game_items WHERE type = '" + type + "' ORDER BY level, game_id";
    PGresult* result = db->query(query);

    if (!result) return games;

    int rowCount = PQntuples(result);
    for (int i = 0; i < rowCount; i++) {
        Game game;
        if (parseGameFromRow(result, i, game)) {
            games.push_back(game);
        }
    }
    PQclear(result);
    return games;
}

Game GameRepository::getGameById(int id) {
    Game game;
    if (!db || !db->isConnected()) return game;

    std::string query = "SELECT * FROM game_items WHERE game_id = " + std::to_string(id);
    PGresult* result = db->query(query);

    if (!result) return game;

    if (PQntuples(result) > 0) {
        parseGameFromRow(result, 0, game);
    }
    PQclear(result);
    return game;
}

} // namespace server
