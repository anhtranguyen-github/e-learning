#ifndef EXAM_LOADER_H
#define EXAM_LOADER_H

#include "server/database.h"
#include "server/model/exam.h"
#include "common/payloads.h"
#include <string>
#include <vector>
#include <memory>
#include <sstream>

namespace server {

// Repository class for accessing exams from database
class ExamRepository {
private:
    std::shared_ptr<Database> db;
    bool parseExamFromRow(PGresult* result, int row, Exam& exam) const;
    // Helper to parse JSON array from PostgreSQL JSONB field
    std::vector<std::string> parseJsonArray(const std::string& jsonStr) const;

public:
    ExamRepository(std::shared_ptr<Database> database);

    // Load all exams from database
    ExamList loadAllExams();

    // Load a specific exam by ID with full content
    Exam loadExamById(int examId);

    // Load exams filtered by lesson ID, type, and/or level
    ExamList loadExamsByFilter(int lessonId, const std::string& type, const std::string& level);
};

} // namespace server

#endif // EXAM_REPOSITORY_H
