#include "server/model/game.h"

namespace server {

Game::Game(int id, const std::string& type, const std::string& level, const std::string& questionJson)
    : m_id(id), m_type(type), m_level(level), m_questionJson(questionJson) {}

Game::Game() : m_id(0) {}

int Game::getId() const { return m_id; }
std::string Game::getType() const { return m_type; }
std::string Game::getLevel() const { return m_level; }
std::string Game::getQuestionJson() const { return m_questionJson; }

void Game::setId(int id) { m_id = id; }
void Game::setType(const std::string& type) { m_type = type; }
void Game::setLevel(const std::string& level) { m_level = level; }
void Game::setQuestionJson(const std::string& json) { m_questionJson = json; }

Payloads::GameDataDTO Game::toDTO() const {
    Payloads::GameDataDTO dto;
    dto.id = std::to_string(m_id);
    dto.type = m_type;
    dto.level = m_level;
    dto.questionJson = m_questionJson;
    return dto;
}

} // namespace server
