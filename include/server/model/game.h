#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>
#include "common/payloads.h"

namespace server {

class Game {
public:
    Game(int id, const std::string& type, const std::string& level, const std::string& questionJson);
    Game();

    int getId() const;
    std::string getType() const;
    std::string getLevel() const;
    std::string getQuestionJson() const;

    void setId(int id);
    void setType(const std::string& type);
    void setLevel(const std::string& level);
    void setQuestionJson(const std::string& json);

    Payloads::GameDataDTO toDTO() const;

private:
    int m_id;
    std::string m_type;
    std::string m_level;
    std::string m_questionJson;
};

} // namespace server

#endif // GAME_H
