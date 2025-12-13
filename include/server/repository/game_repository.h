#ifndef GAME_REPOSITORY_H
#define GAME_REPOSITORY_H

#include <memory>
#include <vector>
#include <string>
#include "server/database.h"
#include "server/model/game.h"
#include <postgresql/libpq-fe.h>

namespace server {

class GameRepository {
public:
    explicit GameRepository(std::shared_ptr<Database> database);
    
    std::vector<std::string> getGameTypes();
    std::vector<Game> getLevelsByType(const std::string& type);
    Game getGameById(int id);

private:
    std::shared_ptr<Database> db;
    bool parseGameFromRow(PGresult* result, int row, Game& game) const;
};

} // namespace server

#endif // GAME_REPOSITORY_H
