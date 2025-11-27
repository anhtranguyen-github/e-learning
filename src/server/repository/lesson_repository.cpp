#include "server/repository/lesson_repository.h"
#include "common/logger.h"
#include <sstream>
#include <algorithm>
#include <json/json.h>

namespace server {

// ============================================================================
// LessonRepository Implementation
// ============================================================================

LessonRepository::LessonRepository(std::shared_ptr<Database> database) : db(database) {
}

std::vector<std::string> LessonRepository::parseJsonArray(const std::string& jsonStr) const {
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

bool LessonRepository::parseLessonFromRow(PGresult* result, int row, Lesson& lesson) const {
    try {
        lesson.setLessonId(std::stoi(PQgetvalue(result, row, PQfnumber(result, "lesson_id"))));
        lesson.setTitle(PQgetvalue(result, row, PQfnumber(result, "title")));
        lesson.setTopic(PQgetvalue(result, row, PQfnumber(result, "topic")));
        lesson.setLevel(PQgetvalue(result, row, PQfnumber(result, "level")));
        lesson.setVideoUrl(PQgetvalue(result, row, PQfnumber(result, "video_url")));
        lesson.setAudioUrl(PQgetvalue(result, row, PQfnumber(result, "audio_url")));
        lesson.setTextContent(PQgetvalue(result, row, PQfnumber(result, "text_content")));

        // Parse JSON arrays
        std::string vocabJson = PQgetvalue(result, row, PQfnumber(result, "vocabulary"));
        std::string grammarJson = PQgetvalue(result, row, PQfnumber(result, "grammar"));
        lesson.setVocabulary(parseJsonArray(vocabJson));
        lesson.setGrammar(parseJsonArray(grammarJson));

        return true;
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Error parsing lesson from row: " + std::string(e.what()));
        }
        return false;
    }
}

LessonList LessonRepository::loadAllLessons() {
    LessonList lessonList;
    
    if (!db) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database connection is null in loadAllLessons");
        }
        return lessonList;
    }
    
    if (!db->isConnected()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database not connected in loadAllLessons");
        }
        return lessonList;
    }
    
    std::string query = "SELECT * FROM lessons ORDER BY topic, level, title";
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Executing query in loadAllLessons: " + query);
    }
    
    try {
        auto startTime = std::chrono::steady_clock::now();
        PGresult* result = db->query(query);  // Changed to raw pointer
        auto endTime = std::chrono::steady_clock::now();
        
        if (logger::serverLogger) {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            logger::serverLogger->debug("[DEBUG] Query executed in " + std::to_string(duration) + "ms");
        }
        
        if (!result) {
            if (logger::serverLogger) {
                logger::serverLogger->error("[ERROR] Query returned null result in loadAllLessons");
            }
            return lessonList;
        }
        
        int rowCount = PQntuples(result);  // Removed ->get()
        
        if (logger::serverLogger) {
            logger::serverLogger->debug("[DEBUG] Found " + std::to_string(rowCount) + " lessons in database");
        }
        
        int successCount = 0;
        for (int i = 0; i < rowCount; i++) {
            try {
                Lesson lesson;
                if (parseLessonFromRow(result, i, lesson)) {  // Removed ->get()
                    lessonList.addLesson(lesson);
                    successCount++;
                }
            } catch (const std::exception& e) {
                if (logger::serverLogger) {
                    logger::serverLogger->error("[ERROR] Error parsing lesson at row " + std::to_string(i) + 
                                              ": " + e.what());
                }
            }
        }
        
        if (logger::serverLogger) {
            logger::serverLogger->info("[INFO] Successfully loaded " + std::to_string(successCount) + 
                                     " out of " + std::to_string(rowCount) + " lessons");
        }
        
        PQclear(result);  // Clean up result
        
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Exception in loadAllLessons: " + std::string(e.what()));
        }
        throw;
    }
    
    return lessonList;
}

Lesson LessonRepository::loadLessonById(int lessonId) {
    Lesson lesson;
    
    if (!db) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database connection is null in loadLessonById");
        }
        return lesson;
    }
    
    if (!db->isConnected()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database not connected in loadLessonById");
        }
        return lesson;
    }
    
    std::string query = "SELECT * FROM lessons WHERE lesson_id = " + std::to_string(lessonId);
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Executing query in loadLessonById: " + query);
    }
    
    try {
        auto startTime = std::chrono::steady_clock::now();
        PGresult* result = db->query(query);  // Changed to raw pointer
        auto endTime = std::chrono::steady_clock::now();
        
        if (logger::serverLogger) {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            logger::serverLogger->debug("[DEBUG] Query executed in " + std::to_string(duration) + "ms");
        }
        
        if (!result) {
            if (logger::serverLogger) {
                logger::serverLogger->error("[ERROR] Query returned null result in loadLessonById");
            }
            return lesson;
        }
        
        int rowCount = PQntuples(result);  // Removed ->get()
        
        if (logger::serverLogger) {
            logger::serverLogger->debug("[DEBUG] Found " + std::to_string(rowCount) + " lessons in database");
        }
        
        if (rowCount == 0) {
            if (logger::serverLogger) {
                logger::serverLogger->warn("[WARN] Lesson " + std::to_string(lessonId) + " not found");
            }
            PQclear(result);
            return lesson;
        }
        
        try {
            if (parseLessonFromRow(result, 0, lesson)) {  // Removed ->get()
                if (logger::serverLogger) {
                    logger::serverLogger->info("[INFO] Successfully loaded lesson " + std::to_string(lessonId));
                }
            } else {
                if (logger::serverLogger) {
                    logger::serverLogger->error("[ERROR] Error parsing lesson " + std::to_string(lessonId));
                }
            }
        } catch (const std::exception& e) {
            if (logger::serverLogger) {
                logger::serverLogger->error("[ERROR] Error parsing lesson " + std::to_string(lessonId) + 
                                          ": " + e.what());
            }
        }
        
        PQclear(result);  // Clean up result
        
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Exception in loadLessonById: " + std::string(e.what()));
        }
        throw;
    }
    
    return lesson;
}

LessonList LessonRepository::loadLessonsByFilter(const std::string& topic, const std::string& level) {
    LessonList lessonList;
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] loadLessonsByFilter called with topic='" + topic +
                                    "', level='" + level + "'");
    }
    
    if (!db) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database connection is null in loadLessonsByFilter");
        }
        return lessonList;
    }
    
    if (!db->isConnected()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database not connected in loadLessonsByFilter");
        }
        return lessonList;
    }
    
    // If no filters, delegate to loadAllLessons
    if (topic.empty() && level.empty()) {
        if (logger::serverLogger) {
            logger::serverLogger->debug("[DEBUG] No filters provided, delegating to loadAllLessons");
        }
        return loadAllLessons();
    }
    
    std::string query = "SELECT * FROM lessons";
    std::vector<std::string> conditions;
    
    if (!topic.empty()) {
        conditions.push_back("topic = '" + topic + "'");
    }
    
    if (!level.empty()) {
        conditions.push_back("level = '" + level + "'");
    }
    
    if (!conditions.empty()) {
        query += " WHERE " + conditions[0];
        for (size_t i = 1; i < conditions.size(); ++i) {
            query += " AND " + conditions[i];
        }
    }
    
    query += " ORDER BY topic, level, title";
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Executing query in loadLessonsByFilter: " + query);
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
                logger::serverLogger->error("[ERROR] Query returned null result in loadLessonsByFilter");
            }
            return lessonList;
        }
        
        int rowCount = PQntuples(result);
        
        if (logger::serverLogger) {
            logger::serverLogger->debug("[DEBUG] Found " + std::to_string(rowCount) + " lessons in database");
        }
        
        for (int i = 0; i < rowCount; ++i) {
            try {
                Lesson lesson;
                if (parseLessonFromRow(result, i, lesson)) {
                    lessonList.addLesson(lesson);
                }
            } catch (const std::exception& e) {
                if (logger::serverLogger) {
                    logger::serverLogger->error("[ERROR] Error parsing lesson at row " + std::to_string(i) +
                                                ": " + e.what());
                }
            }
        }
        
        PQclear(result);
        
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Exception in loadLessonsByFilter: " + std::string(e.what()));
        }
        throw;
    }
    
    return lessonList;
}

}  // namespace server