#include "server/repository/result_repository.h"
#include "common/logger.h"
#include "common/utils.h"
#include <json/json.h>
#include <iostream>

namespace server {

ResultRepository::ResultRepository(std::shared_ptr<Database> database) : db(database) {}

bool ResultRepository::saveResult(int userId, const std::string& targetType, int targetId, 
                                double score, const std::string& userAnswer, 
                                const std::string& feedback, const std::string& status) {
    std::string query = "INSERT INTO results (user_id, target_type, target_id, score, user_answer, feedback, status) VALUES (" +
                        std::to_string(userId) + ", '" + targetType + "', " + std::to_string(targetId) +
                        ", " + std::to_string(score) + ", '" + userAnswer + "', '" + feedback + "', '" + status + "')";
    
    return db->execute(query);
}

bool ResultRepository::updateResult(int resultId, double score, const std::string& feedback, const std::string& status) {
    std::string query = "UPDATE results SET score = " + std::to_string(score) + 
                        ", feedback = '" + feedback + "', status = '" + status + 
                        "', graded_at = CURRENT_TIMESTAMP WHERE result_id = " + std::to_string(resultId);
    
    return db->execute(query);
}

bool ResultRepository::getResult(int userId, const std::string& targetType, int targetId, 
                               double& score, std::string& feedback, std::string& status) {
    std::string query = "SELECT score, feedback, status FROM results WHERE user_id = " + std::to_string(userId) +
                        " AND target_type = '" + targetType + "' AND target_id = " + std::to_string(targetId);

    PGresult* result = db->query(query);
    
    if (result && PQntuples(result) > 0) {
        score = std::stod(PQgetvalue(result, 0, 0));
        feedback = PQgetvalue(result, 0, 1);
        status = PQgetvalue(result, 0, 2);
        PQclear(result);
        return true;
    }
    
    if (result) PQclear(result);
    return false;
}

std::vector<Payloads::ResultSummaryDTO> ResultRepository::getResultsByUser(int userId, const std::string& targetType) {
    std::vector<Payloads::ResultSummaryDTO> results;
    
    // Use DISTINCT ON to get only the latest result per target_type and target_id
    // We order by target_type, target_id, and submitted_at DESC to ensure the first row is the latest
    std::string query = "SELECT DISTINCT ON (r.target_type, r.target_id) "
                        "r.target_id, r.score, r.status, r.feedback, r.target_type, "
                        "COALESCE(e.title, ex.title) as title "
                        "FROM results r "
                        "LEFT JOIN exams e ON r.target_type = 'exam' AND r.target_id = e.exam_id "
                        "LEFT JOIN exercises ex ON r.target_type = 'exercise' AND r.target_id = ex.exercise_id "
                        "WHERE r.user_id = " + std::to_string(userId);

    if (!targetType.empty()) {
        query += " AND r.target_type = '" + targetType + "'";
    }

    // Important: ORDER BY must start with the columns in DISTINCT ON
    query += " ORDER BY r.target_type, r.target_id, r.submitted_at DESC";

    if (logger::serverLogger) logger::serverLogger->debug("Executing query: " + query);

    PGresult* result = db->query(query);
    
    if (result) {
        int nTuples = PQntuples(result);
        if (logger::serverLogger) logger::serverLogger->debug("Query returned " + std::to_string(nTuples) + " rows");

        for (int i = 0; i < nTuples; ++i) {
            Payloads::ResultSummaryDTO dto;
            
            char* val = PQgetvalue(result, i, 0);
            dto.targetId = val ? val : "";
            
            val = PQgetvalue(result, i, 1);
            dto.score = val ? val : "0";
            
            val = PQgetvalue(result, i, 2);
            dto.status = val ? val : "";
            
            val = PQgetvalue(result, i, 3);
            dto.feedback = val ? val : "";

            val = PQgetvalue(result, i, 4);
            dto.targetType = val ? val : "";

            val = PQgetvalue(result, i, 5);
            dto.title = val ? val : "Unknown Title";
            
            results.push_back(dto);
        }
        PQclear(result);
    } else {
        if (logger::serverLogger) logger::serverLogger->error("Query returned null result");
    }
    
    return results;
}

std::vector<Payloads::PendingSubmissionDTO> ResultRepository::getPendingSubmissions() {
    std::vector<Payloads::PendingSubmissionDTO> submissions;
    // Join with users and exams/exercises to get more info
    // For simplicity, we'll just get the raw result data and user name for now
    // A proper implementation would join with exams/exercises tables to get titles
    
    std::string query = "SELECT r.result_id, u.username, r.target_type, r.target_id, r.submitted_at, r.user_answer "
                        "FROM results r "
                        "JOIN users u ON r.user_id = u.user_id "
                        "WHERE r.status = 'pending' "
                        "ORDER BY r.submitted_at ASC";

    PGresult* result = db->query(query);
    
    if (result) {
        for (int i = 0; i < PQntuples(result); ++i) {
            Payloads::PendingSubmissionDTO dto;
            dto.resultId = PQgetvalue(result, i, 0);
            dto.userName = PQgetvalue(result, i, 1);
            dto.targetType = PQgetvalue(result, i, 2);
            dto.targetTitle = "ID: " + std::string(PQgetvalue(result, i, 3)); // Placeholder for title
            dto.submittedAt = PQgetvalue(result, i, 4);
            dto.userAnswer = PQgetvalue(result, i, 5);
            submissions.push_back(dto);
        }
        PQclear(result);
    }
    
    return submissions;
}

bool ResultRepository::getResultDetail(int userId, const std::string& targetType, int targetId, Payloads::ResultDetailDTO& detail) {
    // 1. Fetch result data
    std::string query = "SELECT score, feedback, user_answer FROM results WHERE user_id = " + std::to_string(userId) +
                        " AND target_type = '" + targetType + "' AND target_id = " + std::to_string(targetId) +
                        " ORDER BY submitted_at DESC LIMIT 1";
    
    PGresult* res = db->query(query);
    if (!res || PQntuples(res) == 0) {
        if (res) PQclear(res);
        return false;
    }

    detail.targetId = std::to_string(targetId);
    detail.targetType = targetType;
    detail.score = PQgetvalue(res, 0, 0);
    detail.feedback = PQgetvalue(res, 0, 1);
    std::string userAnswerStr = PQgetvalue(res, 0, 2);
    PQclear(res);

    auto userAnswers = utils::split(userAnswerStr, '^');

    // 2. Fetch content
    std::string contentQuery;
    if (targetType == "exercise") {
        contentQuery = "SELECT title, questions FROM exercises WHERE exercise_id = " + std::to_string(targetId);
    } else if (targetType == "exam") {
        contentQuery = "SELECT title, question FROM exams WHERE exam_id = " + std::to_string(targetId);
    } else {
        return false;
    }

    res = db->query(contentQuery);
    if (!res || PQntuples(res) == 0) {
        if (res) PQclear(res);
        return false;
    }

    detail.title = PQgetvalue(res, 0, 0);
    std::string jsonContent = PQgetvalue(res, 0, 1);
    PQclear(res);

    // 3. Parse JSON
    Json::Value root;
    Json::Reader reader;
    if (reader.parse(jsonContent, root)) {
        // Handle different structures
        // Exercises: root is array of questions
        // Exams: root might be object with "questions" array or just array
        
        const Json::Value& questions = (root.isArray()) ? root : root["questions"];
        
        if (questions.isArray()) {
            for (unsigned int i = 0; i < questions.size(); ++i) {
                Payloads::QuestionResultDTO qDto;
                qDto.questionText = questions[i]["question"].asString();
                qDto.correctAnswer = questions[i]["answer"].asString(); // Or "correct_answer"
                
                if (i < userAnswers.size()) {
                    qDto.userAnswer = userAnswers[i];
                }
                
                // Simple status check (case insensitive?)
                // Ideally, use the score per question if available, but we only have total score.
                // So we can just compare strings for display purposes.
                if (qDto.userAnswer == qDto.correctAnswer) {
                    qDto.status = "correct";
                } else {
                    qDto.status = "incorrect";
                }
                
                detail.questions.push_back(qDto);
            }
        }
    }

    return true;
}

bool ResultRepository::hasResult(int userId, const std::string& targetType, int targetId) {
    std::string query = "SELECT 1 FROM results WHERE user_id = " + std::to_string(userId) +
                        " AND target_type = '" + targetType + "' AND target_id = " + std::to_string(targetId) + " LIMIT 1";
    
    PGresult* res = db->query(query);
    bool exists = (res && PQntuples(res) > 0);
    if (res) PQclear(res);
    return exists;
}

} // namespace server
