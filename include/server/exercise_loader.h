#ifndef EXERCISE_LOADER_H
#define EXERCISE_LOADER_H

#include "server/database.h"
#include <string>
#include <vector>
#include <memory>
#include <sstream>

namespace server {

// Exercise content types for selective loading
enum class ExerciseType {
    QUESTION,
    OPTIONS,
    ANSWER,
    EXPLANATION,
    FULL  // All content
};

// Represents a single exercise with all its content
class Exercise {
private:
    int exerciseId;
    int lessonId;
    std::string title;
    std::string type; // e.g., "multiple_choice", "fill_in_the_blank"
    std::string level;
    std::string question;
    std::vector<std::string> options;
    std::string answer;
    std::string explanation;

public:
    Exercise() : exerciseId(-1), lessonId(-1) {}
    
    Exercise(int id, int lId, const std::string& t, const std::string& ty, const std::string& lv)
        : exerciseId(id), lessonId(lId), title(t), type(ty), level(lv) {}

    // Getters
    int getExerciseId() const { return exerciseId; }
    int getLessonId() const { return lessonId; }
    std::string getTitle() const { return title; }
    std::string getType() const { return type; }
    std::string getLevel() const { return level; }
    std::string getQuestion() const { return question; }
    std::vector<std::string> getOptions() const { return options; }
    std::string getAnswer() const { return answer; }
    std::string getExplanation() const { return explanation; }

    // Setters
    void setExerciseId(int id) { exerciseId = id; }
    void setLessonId(int id) { lessonId = id; }
    void setTitle(const std::string& t) { title = t; }
    void setType(const std::string& ty) { type = ty; }
    void setLevel(const std::string& l) { level = l; }
    void setQuestion(const std::string& q) { question = q; }
    void setOptions(const std::vector<std::string>& opts) { options = opts; }
    void setAnswer(const std::string& ans) { answer = ans; }
    void setExplanation(const std::string& exp) { explanation = exp; }

    // Serialize only the requested content type for network transmission
    std::string serializeForNetwork(ExerciseType type) const;

    // Serialize metadata (id, title, type, level) for exercise list
    std::string serializeMetadata() const;
};

// Container for multiple exercises with filtering capabilities
class ExerciseList {
private:
    std::vector<Exercise> exercises;

public:
    ExerciseList() = default;

    // Add an exercise to the list
    void addExercise(const Exercise& exercise) {
        exercises.push_back(exercise);
    }

    // Get all exercises
    const std::vector<Exercise>& getExercises() const { return exercises; }

    // Filter exercises by type, level, and/or lessonId
    // Empty string/negative lessonId means no filter for that field
    ExerciseList filter(const std::string& type, const std::string& level, int lessonId) const;

    // Serialize all exercises metadata for network transmission
    // Format: <count>;<exercise1_meta>;<exercise2_meta>;...
    std::string serializeForNetwork() const;

    // Get exercise count
    size_t count() const { return exercises.size(); }

    // Clear all exercises
    void clear() { exercises.clear(); }
};

// Loader class for loading exercises from database
class ExerciseLoader {
private:
    std::shared_ptr<Database> db;
    bool parseExerciseFromRow(PGresult* result, int row, Exercise& exercise) const;
    // Helper to parse JSON array from PostgreSQL JSONB field
    std::vector<std::string> parseJsonArray(const std::string& jsonStr) const;

public:
    ExerciseLoader(std::shared_ptr<Database> database);

    // Load all exercises from database
    ExerciseList loadAllExercises();

    // Load a specific exercise by ID with full content
    Exercise loadExerciseById(int exerciseId);

    // Load exercises filtered by type, level, and/or lessonId
    ExerciseList loadExercisesByFilter(const std::string& type, const std::string& level, int lessonId);
};

} // namespace server

#endif // EXERCISE_LOADER_H
