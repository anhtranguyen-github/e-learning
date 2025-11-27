#include "server/repository/exercise_repository.h"
#include "common/logger.h"
#include <sstream>
#include <algorithm>
#include <json/json.h>

namespace server {

// ============================================================================
// ExerciseRepository Implementation
// ============================================================================

ExerciseRepository::ExerciseRepository(std::shared_ptr<Database> database) : db(database) {
}

std::vector<std::string> ExerciseRepository::parseJsonArray(const std::string& jsonStr) const {
    std::vector<std::string> result;
    
    if (jsonStr.empty() || jsonStr == "null") {
        return result;
    }
    
    try {
        Json::Value root;
        Json::Reader reader;
        
        if (reader.parse(jsonStr, root) && root.isArray()) {
            for (const auto& item : root) {
                if (item.isString()) {
                    result.push_back(item.asString());
                }
            }
        }
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Error parsing JSON array: " + std::string(e.what()));
        }
    }
    
    return result;
}

bool ExerciseRepository::parseExerciseFromRow(PGresult* result, int row, Exercise& exercise) const {
    try {
        int col_exercise_id = PQfnumber(result, "exercise_id");
        if (logger::serverLogger) { logger::serverLogger->debug("[DEBUG] Column exercise_id index: " + std::to_string(col_exercise_id)); }
        exercise.setExerciseId(std::stoi(PQgetvalue(result, row, col_exercise_id)));
        
        // Handle nullable lesson_id
        int col_lesson_id = PQfnumber(result, "lesson_id");
        if (logger::serverLogger) { logger::serverLogger->debug("[DEBUG] Column lesson_id index: " + std::to_string(col_lesson_id)); }
        if (PQgetisnull(result, row, col_lesson_id)) {
            exercise.setLessonId(0); // Default value for null lesson_id
            if (logger::serverLogger) { logger::serverLogger->debug("[DEBUG] lesson_id is null, setting to 0"); }
        } else {
            exercise.setLessonId(std::stoi(PQgetvalue(result, row, col_lesson_id)));
            if (logger::serverLogger) { logger::serverLogger->debug("[DEBUG] lesson_id: " + std::string(PQgetvalue(result, row, col_lesson_id))); }
        }

        // Handle non-nullable title
        int col_title = PQfnumber(result, "title");
        if (logger::serverLogger) { logger::serverLogger->debug("[DEBUG] Column title index: " + std::to_string(col_title)); }
        if (PQgetisnull(result, row, col_title)) {
            if (logger::serverLogger) {
                logger::serverLogger->error("Error parsing exercise: 'title' is unexpectedly null at row " + std::to_string(row));
            }
            return false;
        } else {
            exercise.setTitle(PQgetvalue(result, row, col_title));
            if (logger::serverLogger) { logger::serverLogger->debug("[DEBUG] title: " + std::string(PQgetvalue(result, row, col_title))); }
        }
        
        // Handle nullable type
        int col_type = PQfnumber(result, "type");
        if (logger::serverLogger) { logger::serverLogger->debug("[DEBUG] Column type index: " + std::to_string(col_type)); }
        if (PQgetisnull(result, row, col_type)) {
            exercise.setType(""); // Default to empty string for null type
            if (logger::serverLogger) { logger::serverLogger->debug("[DEBUG] type is null, setting to empty string"); }
        } else {
            exercise.setType(PQgetvalue(result, row, col_type));
            if (logger::serverLogger) { logger::serverLogger->debug("[DEBUG] type: " + std::string(PQgetvalue(result, row, col_type))); }
        }
        
        // Handle nullable level
        if (PQgetisnull(result, row, PQfnumber(result, "level"))) {
            exercise.setLevel(""); // Default to empty string for null level
        } else {
            exercise.setLevel(PQgetvalue(result, row, PQfnumber(result, "level")));
        }

        // Handle non-nullable question
        if (PQgetisnull(result, row, PQfnumber(result, "question"))) {
            if (logger::serverLogger) {
                logger::serverLogger->error("Error parsing exercise: 'question' is unexpectedly null at row " + std::to_string(row));
            }
            return false;
        } else {
            exercise.setQuestion(PQgetvalue(result, row, PQfnumber(result, "question")));
        }
        
        // Handle nullable options (JSONB)
        std::string optionsJson;
        if (PQgetisnull(result, row, PQfnumber(result, "options"))) {
            optionsJson = "[]"; // Default to empty JSON array for null options
        } else {
            optionsJson = PQgetvalue(result, row, PQfnumber(result, "options"));
        }
        exercise.setOptions(parseJsonArray(optionsJson));

        // Handle nullable answer
        if (PQgetisnull(result, row, PQfnumber(result, "answer"))) {
            exercise.setAnswer(""); // Default to empty string for null answer
        } else {
            exercise.setAnswer(PQgetvalue(result, row, PQfnumber(result, "answer")));
        }
        
        // Handle nullable explanation
        if (PQgetisnull(result, row, PQfnumber(result, "explanation"))) {
            exercise.setExplanation(""); // Default to empty string for null explanation
        } else {
            exercise.setExplanation(PQgetvalue(result, row, PQfnumber(result, "explanation")));
        }


        return true;
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Error parsing exercise from row: " + std::string(e.what()));
        }
        return false;
    }
}

ExerciseList ExerciseRepository::loadAllExercises() {
    ExerciseList exerciseList;
    
    if (!db) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database connection is null in loadAllExercises");
        }
        return exerciseList;
    }
    
    if (!db->isConnected()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database not connected in loadAllExercises");
        }
        return exerciseList;
    }
    
    std::string query = "SELECT * FROM exercises ORDER BY lesson_id, type, level, title";
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Executing query in loadAllExercises: " + query);
    }
    
    try {
        auto startTime = std::chrono::steady_clock::now();
        PGresult* result = db->query(query);
        auto endTime = std::chrono::steady_clock::now();
        
        if (logger::serverLogger) {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            logger::serverLogger->debug("[DEBUG] Query executed in " + std::to_string(duration) + "ms");
        }
        
        if (!result) {
            if (logger::serverLogger) {
                logger::serverLogger->error("[ERROR] Query returned null result in loadAllExercises");
            }
            return exerciseList;
        }
        
        int rowCount = PQntuples(result);
        
        if (logger::serverLogger) {
            logger::serverLogger->debug("[DEBUG] Found " + std::to_string(rowCount) + " exercises in database");
        }
        
        int successCount = 0;
        for (int i = 0; i < rowCount; i++) {
            try {
                Exercise exercise;
                if (parseExerciseFromRow(result, i, exercise)) {
                    exerciseList.addExercise(exercise);
                    successCount++;
                    if (logger::serverLogger) {
                        logger::serverLogger->debug("[DEBUG] Successfully loaded exercise: " + exercise.getTitle() + " (ID: " + std::to_string(exercise.getExerciseId()) + ")");
                    }
                }
            } catch (const std::exception& e) {
                if (logger::serverLogger) {
                    logger::serverLogger->error("[ERROR] Error parsing exercise at row " + std::to_string(i) +
                                              ": " + e.what());
                }
            }
        }
        
        if (logger::serverLogger) {
            logger::serverLogger->info("[INFO] Successfully loaded " + std::to_string(successCount) +
                                     " out of " + std::to_string(rowCount) + " exercises");
        }
        
        PQclear(result);
        
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Exception in loadAllExercises: " + std::string(e.what()));
        }
        throw;
    }
    
    return exerciseList;
}

Exercise ExerciseRepository::loadExerciseById(int exerciseId) {
    Exercise exercise;
    
    if (!db) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database connection is null in loadExerciseById");
        }
        return exercise;
    }
    
    if (!db->isConnected()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database not connected in loadExerciseById");
        }
        return exercise;
    }
    
    std::string query = "SELECT * FROM exercises WHERE exercise_id = " + std::to_string(exerciseId);
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Executing query in loadExerciseById: " + query);
    }
    
    try {
        auto startTime = std::chrono::steady_clock::now();
        PGresult* result = db->query(query);
        auto endTime = std::chrono::steady_clock::now();
        
        if (logger::serverLogger) {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            logger::serverLogger->debug("[DEBUG] Query executed in " + std::to_string(duration) + "ms");
        }
        
        if (!result) {
            if (logger::serverLogger) {
                logger::serverLogger->error("[ERROR] Query returned null result in loadExerciseById");
            }
            return exercise;
        }
        
        int rowCount = PQntuples(result);
        
        if (logger::serverLogger) {
            logger::serverLogger->debug("[DEBUG] Found " + std::to_string(rowCount) + " exercises in database");
        }
        
        if (rowCount == 0) {
            if (logger::serverLogger) {
                logger::serverLogger->warn("[WARN] Exercise " + std::to_string(exerciseId) + " not found");
            }
            PQclear(result);
            return exercise;
        }
        
        try {
            if (parseExerciseFromRow(result, 0, exercise)) {
                if (logger::serverLogger) {
                    logger::serverLogger->info("[INFO] Successfully loaded exercise " + std::to_string(exerciseId));
                }
            } else {
                if (logger::serverLogger) {
                    logger::serverLogger->error("[ERROR] Error parsing exercise " + std::to_string(exerciseId));
                }
            }
        } catch (const std::exception& e) {
            if (logger::serverLogger) {
                logger::serverLogger->error("[ERROR] Error parsing exercise " + std::to_string(exerciseId) +
                                          ": " + e.what());
            }
        }
        
        PQclear(result);
        
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Exception in loadExerciseById: " + std::string(e.what()));
        }
        throw;
    }
    
    return exercise;
}

ExerciseList ExerciseRepository::loadExercisesByFilter(int lessonId, const std::string& type, const std::string& level) {
    ExerciseList exerciseList;
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] loadExercisesByFilter called with type='" + type +
                                    "', level='" + level + "', lessonId='" + std::to_string(lessonId) + "'");
    }
    
    if (!db) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database connection is null in loadExercisesByFilter");
        }
        return exerciseList;
    }
    
    if (!db->isConnected()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database not connected in loadExercisesByFilter");
        }
        return exerciseList;
    }
    
    // If no filters, delegate to loadAllExercises
    if (type.empty() && level.empty() && lessonId == -1) {
        if (logger::serverLogger) {
            logger::serverLogger->debug("[DEBUG] No filters provided, delegating to loadAllExercises");
        }
        return loadAllExercises();
    }
    
    std::string query = "SELECT * FROM exercises";
    std::vector<std::string> conditions;
    
    if (!type.empty()) {
        conditions.push_back("type = '" + type + "'");
    }
    
    if (!level.empty()) {
        conditions.push_back("level = '" + level + "'");
    }

    if (lessonId != -1) {
        conditions.push_back("lesson_id = " + std::to_string(lessonId));
    }
    
    if (!conditions.empty()) {
        query += " WHERE " + conditions[0];
        for (size_t i = 1; i < conditions.size(); ++i) {
            query += " AND " + conditions[i];
        }
    }
    
    query += " ORDER BY lesson_id, type, level, title";
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Executing query in loadExercisesByFilter: " + query);
    }
    
    try {
        auto startTime = std::chrono::steady_clock::now();
        PGresult* result = db->query(query);
        auto endTime = std::chrono::steady_clock::now();
        
        if (logger::serverLogger) {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            logger::serverLogger->debug("[DEBUG] Query executed in " + std::to_string(duration) + "ms");
        }
        
        if (!result) {
            if (logger::serverLogger) {
                logger::serverLogger->error("[ERROR] Query returned null result in loadExercisesByFilter");
            }
            return exerciseList;
        }
        
        int rowCount = PQntuples(result);
        
        if (logger::serverLogger) {
            logger::serverLogger->debug("[DEBUG] Found " + std::to_string(rowCount) + " exercises in database");
        }
        
        for (int i = 0; i < rowCount; ++i) {
            try {
                Exercise exercise;
                if (parseExerciseFromRow(result, i, exercise)) {
                    exerciseList.addExercise(exercise);
                }
            } catch (const std::exception& e) {
                if (logger::serverLogger) {
                    logger::serverLogger->error("[ERROR] Error parsing exercise at row " + std::to_string(i) +
                                                ": " + e.what());
                }
            }
        }
        
        PQclear(result);
        
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Exception in loadExercisesByFilter: " + std::string(e.what()));
        }
        throw;
    }
    
    return exerciseList;
}

} // namespace server