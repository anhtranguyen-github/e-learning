#include "server/repository/exam_repository.h"
#include "common/logger.h"
#include <sstream>
#include <algorithm>
#include <json/json.h>

namespace server {

// ============================================================================
// ExamLoader Implementation
// ============================================================================

ExamRepository::ExamRepository(std::shared_ptr<Database> database) : db(database) {
}

std::vector<std::string> ExamRepository::parseJsonArray(const std::string& jsonStr) const {
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

bool ExamRepository::parseExamFromRow(PGresult* result, int row, Exam& exam) const {
    try {
        exam.setExamId(std::stoi(PQgetvalue(result, row, PQfnumber(result, "exam_id"))));
        exam.setLessonId(std::stoi(PQgetvalue(result, row, PQfnumber(result, "lesson_id"))));
        exam.setTitle(PQgetvalue(result, row, PQfnumber(result, "title")));
        exam.setType(PQgetvalue(result, row, PQfnumber(result, "type")));
        exam.setLevel(PQgetvalue(result, row, PQfnumber(result, "level")));
        
        std::string questionsJson = PQgetvalue(result, row, PQfnumber(result, "questions"));
        exam.setQuestions(parseJsonArray(questionsJson));

        return true;
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Error parsing exam from row: " + std::string(e.what()));
        }
        return false;
    }
}

ExamList ExamRepository::loadAllExams() {
    ExamList examList;
    
    if (!db) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database connection is null in loadAllExams");
        }
        return examList;
    }
    
    if (!db->isConnected()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database not connected in loadAllExams");
        }
        return examList;
    }
    
    std::string query = "SELECT * FROM exams ORDER BY lesson_id, type, level, title";
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Executing query in loadAllExams: " + query);
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
                logger::serverLogger->error("[ERROR] Query returned null result in loadAllExams");
            }
            return examList;
        }
        
        int rowCount = PQntuples(result);
        
        if (logger::serverLogger) {
            logger::serverLogger->debug("[DEBUG] Found " + std::to_string(rowCount) + " exams in database");
        }
        
        int successCount = 0;
        for (int i = 0; i < rowCount; i++) {
            try {
                Exam exam;
                if (parseExamFromRow(result, i, exam)) {
                    examList.addExam(exam);
                    successCount++;
                }
            } catch (const std::exception& e) {
                if (logger::serverLogger) {
                    logger::serverLogger->error("[ERROR] Error parsing exam at row " + std::to_string(i) +
                                              ": " + e.what());
                }
            }
        }
        
        if (logger::serverLogger) {
            logger::serverLogger->info("[INFO] Successfully loaded " + std::to_string(successCount) +
                                     " out of " + std::to_string(rowCount) + " exams");
        }
        
        PQclear(result);
        
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Exception in loadAllExams: " + std::string(e.what()));
        }
        throw;
    }
    
    return examList;
}

Exam ExamRepository::loadExamById(int examId) {
    Exam exam;
    
    if (!db) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database connection is null in loadExamById");
        }
        return exam;
    }
    
    if (!db->isConnected()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database not connected in loadExamById");
        }
        return exam;
    }
    
    std::string query = "SELECT * FROM exams WHERE exam_id = " + std::to_string(examId);
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] Executing query in loadExamById: " + query);
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
                logger::serverLogger->error("[ERROR] Query returned null result in loadExamById");
            }
            return exam;
        }
        
        int rowCount = PQntuples(result);
        
        if (logger::serverLogger) {
            logger::serverLogger->debug("[DEBUG] Found " + std::to_string(rowCount) + " exams in database");
        }
        
        if (rowCount == 0) {
            if (logger::serverLogger) {
                logger::serverLogger->warn("[WARN] Exam " + std::to_string(examId) + " not found");
            }
            PQclear(result);
            return exam;
        }
        
        try {
            if (parseExamFromRow(result, 0, exam)) {
                if (logger::serverLogger) {
                    logger::serverLogger->info("[INFO] Successfully loaded exam " + std::to_string(examId));
                }
            } else {
                if (logger::serverLogger) {
                    logger::serverLogger->error("[ERROR] Error parsing exam " + std::to_string(examId));
                }
            }
        } catch (const std::exception& e) {
            if (logger::serverLogger) {
                logger::serverLogger->error("[ERROR] Error parsing exam " + std::to_string(examId) +
                                          ": " + e.what());
            }
        }
        
        PQclear(result);
        
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Exception in loadExamById: " + std::string(e.what()));
        }
        throw;
    }
    
    return exam;
}

ExamList ExamRepository::loadExamsByFilter(int lessonId, const std::string& type, const std::string& level) {
    ExamList examList;
    
    if (logger::serverLogger) {
        logger::serverLogger->debug("[DEBUG] loadExamsByFilter called with type='" + type +
                                    "', level='" + level + "', lessonId='" + std::to_string(lessonId) + "'");
    }
    
    if (!db) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database connection is null in loadExamsByFilter");
        }
        return examList;
    }
    
    if (!db->isConnected()) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Database not connected in loadExamsByFilter");
        }
        return examList;
    }
    
    // If no filters, delegate to loadAllExams
    if (type.empty() && level.empty() && lessonId == -1) {
        if (logger::serverLogger) {
            logger::serverLogger->debug("[DEBUG] No filters provided, delegating to loadAllExams");
        }
        return loadAllExams();
    }
    
    std::string query = "SELECT * FROM exams";
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
        logger::serverLogger->debug("[DEBUG] Executing query in loadExamsByFilter: " + query);
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
                logger::serverLogger->error("[ERROR] Query returned null result in loadExamsByFilter");
            }
            return examList;
        }
        
        int rowCount = PQntuples(result);
        
        if (logger::serverLogger) {
            logger::serverLogger->debug("[DEBUG] Found " + std::to_string(rowCount) + " exams in database");
        }
        
        for (int i = 0; i < rowCount; ++i) {
            try {
                Exam exam;
                if (parseExamFromRow(result, i, exam)) {
                    examList.addExam(exam);
                }
            } catch (const std::exception& e) {
                if (logger::serverLogger) {
                    logger::serverLogger->error("[ERROR] Error parsing exam at row " + std::to_string(i) +
                                                ": " + e.what());
                }
            }
        }
        
        PQclear(result);
        
    } catch (const std::exception& e) {
        if (logger::serverLogger) {
            logger::serverLogger->error("[ERROR] Exception in loadExamsByFilter: " + std::string(e.what()));
        }
        throw;
    }
    
    return examList;
}

} // namespace server