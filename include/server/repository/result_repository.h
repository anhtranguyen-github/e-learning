#ifndef SERVER_REPOSITORY_RESULT_REPOSITORY_H
#define SERVER_REPOSITORY_RESULT_REPOSITORY_H

#include "server/database.h"
#include "common/payloads.h"
#include <memory>
#include <vector>
#include <string>

namespace server {

class ResultRepository {
private:
    std::shared_ptr<Database> db;

public:
    ResultRepository(std::shared_ptr<Database> database);

    // Save a new result
    bool saveResult(int userId, const std::string& targetType, int targetId, 
                   double score, const std::string& userAnswer, 
                   const std::string& feedback, const std::string& status);

    // Update an existing result (for grading)
    bool updateResult(int resultId, double score, const std::string& feedback, const std::string& status, const std::string& gradingDetails = "");

    // Get a specific result
    bool getResult(int userId, const std::string& targetType, int targetId, 
                  double& score, std::string& feedback, std::string& status);

    // Get all results for a user and target type
    std::vector<Payloads::ResultSummaryDTO> getResultsByUser(int userId, const std::string& targetType);

    // Get pending submissions for teachers (legacy)
    std::vector<Payloads::PendingSubmissionDTO> getPendingSubmissions();

    // Get all submissions for teacher review (using SubmissionDTO)
    std::vector<Payloads::SubmissionDTO> getSubmissions();
    bool getSubmissionDetail(int resultId, Payloads::SubmissionDetailDTO& detail);

    // Add feedback to a result
    bool addFeedback(int resultId, const std::string& feedbackContent, const std::string& feedbackType);

    // Get detailed result
    bool getResultDetail(int userId, const std::string& targetType, int targetId, Payloads::ResultDetailDTO& detail);

    // Check if result exists
    bool hasResult(int userId, const std::string& targetType, int targetId);
};


} // namespace server

#endif // SERVER_REPOSITORY_RESULT_REPOSITORY_H
