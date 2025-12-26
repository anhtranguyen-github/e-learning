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
    const char* query = "INSERT INTO results (user_id, target_type, target_id, score, user_answer, feedback, status) VALUES ($1, $2, $3, $4, $5, $6, $7)";
    
    std::string s_userId = std::to_string(userId);
    std::string s_targetId = std::to_string(targetId);
    std::string s_score = std::to_string(score);
    
    const char* params[7];
    params[0] = s_userId.c_str();
    params[1] = targetType.c_str();
    params[2] = s_targetId.c_str();
    params[3] = s_score.c_str();
    params[4] = userAnswer.c_str();
    params[5] = feedback.c_str();
    params[6] = status.c_str();

    PGresult* res = db->execParams(query, 7, params);
    bool success = (res && PQresultStatus(res) == PGRES_COMMAND_OK);
    if (res) PQclear(res);
    return success;
}

bool ResultRepository::updateResult(int resultId, double score, const std::string& feedback, const std::string& status, const std::string& gradingDetails) {
    std::string details = gradingDetails.empty() ? "{}" : gradingDetails;
    const char* query = "UPDATE results SET score = $1, feedback = $2, status = $3, grading_details = $4, graded_at = CURRENT_TIMESTAMP WHERE result_id = $5";
    
    std::string s_score = std::to_string(score);
    std::string s_resultId = std::to_string(resultId);

    const char* params[5];
    params[0] = s_score.c_str();
    params[1] = feedback.c_str();
    params[2] = status.c_str();
    params[3] = details.c_str();
    params[4] = s_resultId.c_str();
    
    PGresult* res = db->execParams(query, 5, params);
    bool success = (res && PQresultStatus(res) == PGRES_COMMAND_OK);
    if (res) PQclear(res);
    return success;
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
    
    // Join with users and exercises/exams tables to get titles and status
    std::string query = "SELECT r.result_id, u.username, r.target_type, r.target_id, r.submitted_at, "
                        "r.user_answer, r.status, "
                        "COALESCE(ex.title, e.title, 'Unknown') as title "
                        "FROM results r "
                        "JOIN users u ON r.user_id = u.user_id "
                        "LEFT JOIN exercises ex ON r.target_type = 'exercise' AND r.target_id = ex.exercise_id "
                        "LEFT JOIN exams e ON r.target_type = 'exam' AND r.target_id = e.exam_id "
                        "ORDER BY r.status DESC, r.submitted_at ASC"; // pending first, then graded

    PGresult* result = db->query(query);
    
    if (result) {
        for (int i = 0; i < PQntuples(result); ++i) {
            Payloads::PendingSubmissionDTO dto;
            dto.resultId = PQgetvalue(result, i, 0);
            dto.userName = PQgetvalue(result, i, 1);
            dto.targetType = PQgetvalue(result, i, 2);
            dto.targetId = PQgetvalue(result, i, 3);
            dto.submittedAt = PQgetvalue(result, i, 4);
            dto.userAnswer = PQgetvalue(result, i, 5);
            dto.status = PQgetvalue(result, i, 6);
            dto.targetTitle = PQgetvalue(result, i, 7);
            submissions.push_back(dto);
        }
        PQclear(result);
    }
    
    return submissions;
}

bool ResultRepository::getResultDetail(int userId, const std::string& targetType, int targetId, Payloads::ResultDetailDTO& detail) {
    // 1. Fetch result data
    std::string query = "SELECT score, feedback, user_answer, grading_details FROM results WHERE user_id = " + std::to_string(userId) +
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
    std::string gradingDetailsStr = PQgetvalue(res, 0, 3) ? PQgetvalue(res, 0, 3) : "";
    PQclear(res);

    auto userAnswers = utils::split(userAnswerStr, '^');

    // 2. Fetch content
    std::string jsonContent;

    if (targetType == "exercise") {
        std::string contentQuery = "SELECT title, question, answer, options, explanation, type FROM exercises WHERE exercise_id = " + std::to_string(targetId);
        res = db->query(contentQuery);
        if (!res || PQntuples(res) == 0) {
            if (res) PQclear(res);
            return false;
        }
        detail.title = PQgetvalue(res, 0, 0);
        std::string questionText = PQgetvalue(res, 0, 1) ? PQgetvalue(res, 0, 1) : "";
        std::string answerText = PQgetvalue(res, 0, 2) ? PQgetvalue(res, 0, 2) : "";
        std::string optionsText = PQgetvalue(res, 0, 3) ? PQgetvalue(res, 0, 3) : "";
        std::string explanationText = PQgetvalue(res, 0, 4) ? PQgetvalue(res, 0, 4) : "";
        std::string typeText = PQgetvalue(res, 0, 5) ? PQgetvalue(res, 0, 5) : "";
        if (typeText == "essay" || typeText == "speaking") {
            jsonContent.clear();
        } else if (!questionText.empty()) {
            Json::Value root(Json::arrayValue);
            Json::Value item;
            item["text"] = questionText;
            item["type"] = typeText;
            item["answer"] = answerText;
            item["explanation"] = explanationText;

            Json::Value opts(Json::arrayValue);
            if (!optionsText.empty()) {
                Json::Value parsedOptions;
                Json::Reader optionsReader;
                if (optionsReader.parse(optionsText, parsedOptions) && parsedOptions.isArray()) {
                    opts = parsedOptions;
                } else {
                    auto splitOpts = utils::split(optionsText, ',');
                    for (const auto& opt : splitOpts) {
                        if (!opt.empty()) {
                            opts.append(opt);
                        }
                    }
                }
            }
            item["options"] = opts;
            root.append(item);
            Json::FastWriter writer;
            jsonContent = writer.write(root);
        }
        PQclear(res);

    } else if (targetType == "exam") {
        std::string contentQuery = "SELECT title, question FROM exams WHERE exam_id = " + std::to_string(targetId);
        res = db->query(contentQuery);
        if (!res || PQntuples(res) == 0) {
            if (res) PQclear(res);
            return false;
        }
        detail.title = PQgetvalue(res, 0, 0);
        jsonContent = PQgetvalue(res, 0, 1) ? PQgetvalue(res, 0, 1) : "";
        PQclear(res);
    } else {
        return false;
    }

    // 3. Parse and Populate
    if (!jsonContent.empty()) {
        Json::Value root;
        Json::Reader reader;
        if (reader.parse(jsonContent, root)) {
            const Json::Value& questions = (root.isArray()) ? root : root["questions"];
            if (questions.isArray() && !questions.empty()) {
                for (unsigned int i = 0; i < questions.size(); ++i) {
                    Payloads::QuestionResultDTO qDto;
                    qDto.questionText = questions[i]["text"].asString();
                    qDto.correctAnswer = questions[i]["answer"].asString();
                    
                    if (i < userAnswers.size()) {
                        qDto.userAnswer = userAnswers[i];
                    }
                    
                    if (qDto.userAnswer == qDto.correctAnswer) {
                        qDto.status = "correct";
                    } else {
                        qDto.status = "incorrect";
                    }
                    
                    detail.questions.push_back(qDto);
                }
            }
        }
    }

    // 4. Merge Grading Details
    if (!gradingDetailsStr.empty()) {
        Json::Value gradingRoot;
        Json::Reader reader;
        if (reader.parse(gradingDetailsStr, gradingRoot)) {
            const Json::Value& items = gradingRoot["items"];
            if (items.isArray()) {
                for (unsigned int i = 0; i < items.size() && i < detail.questions.size(); ++i) {
                     if (items[i].isMember("score")) {
                         detail.questions[i].score = items[i]["score"].asString(); 
                     }
                     if (items[i].isMember("comment")) {
                         detail.questions[i].comment = items[i]["comment"].asString();
                     }
                     // Optional: update status if teacher manually graded? 
                     // For now, keep original status unless we decide teacher overrides it.
                }
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

std::vector<Payloads::SubmissionDTO> ResultRepository::getSubmissions() {
    std::vector<Payloads::SubmissionDTO> submissions;
    
    // Get all submissions with proper joins for titles and scores
    std::string query = "SELECT r.result_id, u.username, r.user_id, r.target_type, r.target_id, r.submitted_at, "
                        "r.user_answer, r.status, r.score, "
                        "COALESCE(ex.title, e.title, 'Unknown') as title, "
                        "COALESCE(ex.lesson_id, e.lesson_id) as lesson_id "
                        "FROM results r "
                        "JOIN users u ON r.user_id = u.user_id "
                        "LEFT JOIN exercises ex ON r.target_type = 'exercise' AND r.target_id = ex.exercise_id "
                        "LEFT JOIN exams e ON r.target_type = 'exam' AND r.target_id = e.exam_id "
                        "ORDER BY CASE WHEN r.status = 'pending' THEN 0 ELSE 1 END, r.submitted_at DESC";

    PGresult* result = db->query(query);
    
    if (result) {
        for (int i = 0; i < PQntuples(result); ++i) {
            Payloads::SubmissionDTO dto;
            dto.resultId = PQgetvalue(result, i, 0);
            dto.studentName = PQgetvalue(result, i, 1);
            dto.studentId = PQgetvalue(result, i, 2);
            dto.targetType = PQgetvalue(result, i, 3);
            dto.targetId = PQgetvalue(result, i, 4);
            dto.submittedAt = PQgetvalue(result, i, 5);
            dto.userAnswer = PQgetvalue(result, i, 6);
            dto.status = PQgetvalue(result, i, 7);
            dto.score = PQgetvalue(result, i, 8) ? PQgetvalue(result, i, 8) : "0";
            dto.targetTitle = PQgetvalue(result, i, 9);
            dto.lessonId = PQgetvalue(result, i, 10) ? PQgetvalue(result, i, 10) : "";
            submissions.push_back(dto);
        }
        PQclear(result);
    }
    
    return submissions;
}

bool ResultRepository::addFeedback(int resultId, const std::string& feedbackContent, const std::string& feedbackType) {
    // feedbackType reserved for future use (e.g., "text" vs "audio")
    (void)feedbackType;
    
    std::string query = "UPDATE results SET feedback = $1, graded_at = CURRENT_TIMESTAMP WHERE result_id = $2";
    
    std::string s_resultId = std::to_string(resultId);
    
    const char* params[2];
    params[0] = feedbackContent.c_str();
    params[1] = s_resultId.c_str();
    
    PGresult* res = db->execParams(query.c_str(), 2, params);
    bool success = (res && PQresultStatus(res) == PGRES_COMMAND_OK);
    if (res) PQclear(res);
    return success;
}

} // namespace server
