#ifndef QUIZ_LOADER_H
#define QUIZ_LOADER_H

#include "server/database.h"
#include <string>
#include <vector>
#include <memory>
#include <sstream>

namespace server {

// Represents a single quiz with its content
class Quiz {
private:
    int quizId;
    std::string title;
    std::string topic;
    std::string level;
    std::vector<std::string> questions;

public:
    Quiz() : quizId(-1) {}

    // Getters
    int getQuizId() const { return quizId; }
    std::string getTitle() const { return title; }
    std::string getTopic() const { return topic; }
    std::string getLevel() const { return level; }
    const std::vector<std::string>& getQuestions() const { return questions; }

    // Setters
    void setQuizId(int id) { quizId = id; }
    void setTitle(const std::string& t) { title = t; }
    void setTopic(const std::string& t) { topic = t; }
    void setLevel(const std::string& l) { level = l; }
    void setQuestions(const std::vector<std::string>& q) { questions = q; }

    // Serialize metadata (id, title, topic, level) for quiz list
    std::string serializeMetadata() const;
};

// Container for multiple quizzes with filtering capabilities
class QuizList {
private:
    std::vector<Quiz> quizzes;

public:
    QuizList() = default;

    // Add a quiz to the list
    void addQuiz(const Quiz& quiz) {
        quizzes.push_back(quiz);
    }

    // Get all quizzes
    const std::vector<Quiz>& getQuizzes() const { return quizzes; }

    // Filter quizzes by topic and/or level
    // Empty string means no filter for that field
    QuizList filter(const std::string& topic, const std::string& level) const;

    // Serialize all quizzes metadata for network transmission
    // Format: <count>;<quiz1_meta>;<quiz2_meta>;...
    std::string serializeForNetwork() const;

    // Get quiz count
    size_t count() const { return quizzes.size(); }

    // Clear all quizzes
    void clear() { quizzes.clear(); }
};

// Loader class for loading quizzes from database
class QuizLoader {
private:
    std::shared_ptr<Database> db;
    bool parseQuizFromRow(PGresult* result, int row, Quiz& quiz) const;
    // Helper to parse JSON array from PostgreSQL JSONB field
    std::vector<std::string> parseJsonArray(const std::string& jsonStr) const;

public:
    QuizLoader(std::shared_ptr<Database> database);

    // Load all quizzes from database
    QuizList loadAllQuizzes();

    // Load a specific quiz by ID with full content
    Quiz loadQuizById(int quizId);

    // Load quizzes filtered by topic and/or level
    QuizList loadQuizzesByFilter(const std::string& topic, const std::string& level);
};

} // namespace server

#endif // QUIZ_LOADER_H
