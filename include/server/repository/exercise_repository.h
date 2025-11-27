#ifndef EXERCISE_LOADER_H
#define EXERCISE_LOADER_H

#include "server/database.h"
#include "server/model/exercise.h"
#include "common/payloads.h"
#include <string>
#include <vector>
#include <memory>
#include <sstream>

namespace server {

// Repository class for accessing exercises from database
class ExerciseRepository {
private:
    std::shared_ptr<Database> db;
    bool parseExerciseFromRow(PGresult* result, int row, Exercise& exercise) const;
    // Helper to parse JSON array from PostgreSQL JSONB field
    std::vector<Question> parseQuestions(const std::string& jsonStr) const;

public:
    ExerciseRepository(std::shared_ptr<Database> database);

    // Load all exercises
    ExerciseList loadAllExercises();

    // Load a specific exercise by ID
    Exercise loadExerciseById(int exerciseId);

    // Load exercises filtered by lesson ID, type, and/or level
    ExerciseList loadExercisesByFilter(int lessonId, const std::string& type, const std::string& level);
};

} // namespace server

#endif // EXERCISE_REPOSITORY_H
