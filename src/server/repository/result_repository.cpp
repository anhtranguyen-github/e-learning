#include "server/repository/result_repository.h"
#include "common/logger.h"
#include "common/utils.h"
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
    std::string query = "SELECT target_id, score, status, feedback FROM results WHERE user_id = " + std::to_string(userId);
    
    if (!targetType.empty()) {
        query += " AND target_type = '" + targetType + "'";
    }

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

} // namespace server
