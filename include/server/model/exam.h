#ifndef SERVER_MODEL_EXAM_H
#define SERVER_MODEL_EXAM_H

#include "common/payloads.h"
#include <string>
#include <vector>

namespace server {

// Exam content types for selective loading
enum class ExamType {
    QUESTIONS,
    FULL  // All content
};

// Represents a single exam with all its content
class Exam {
private:
    int examId;
    int lessonId;
    std::string title;
    std::string type; // e.g., "quiz", "final_exam"
    std::string level;
    std::vector<std::string> questions;

public:
    Exam() : examId(-1), lessonId(-1) {}
    
    Exam(int id, int lId, const std::string& t, const std::string& ty, const std::string& lv)
        : examId(id), lessonId(lId), title(t), type(ty), level(lv) {}

    // Getters
    int getExamId() const { return examId; }
    int getLessonId() const { return lessonId; }
    std::string getTitle() const { return title; }
    std::string getType() const { return type; }
    std::string getLevel() const { return level; }
    std::vector<std::string> getQuestions() const { return questions; }

    // Setters
    void setExamId(int id) { examId = id; }
    void setLessonId(int id) { lessonId = id; }
    void setTitle(const std::string& t) { title = t; }
    void setType(const std::string& ty) { type = ty; }
    void setLevel(const std::string& l) { level = l; }
    void setQuestions(const std::vector<std::string>& q) { questions = q; }

    // Serialize only the requested content type for network transmission
    std::string serializeForNetwork(ExamType type) const;

    // Serialize metadata (id, title, type, level) for exam list
    std::string serializeMetadata() const;

    // Convert to DTOs
    Payloads::ExamDTO toDTO() const;
    Payloads::ExamMetadataDTO toMetadataDTO() const;
};

// Container for multiple exams with filtering capabilities
class ExamList {
private:
    std::vector<Exam> exams;

public:
    ExamList() = default;

    // Add an exam to the list
    void addExam(const Exam& exam) {
        exams.push_back(exam);
    }

    // Get all exams
    const std::vector<Exam>& getExams() const { return exams; }

    // Filter exams by type, level, and/or lessonId
    // Empty string/negative lessonId means no filter for that field
    ExamList filter(const std::string& type, const std::string& level, int lessonId) const;

    // Serialize all exams metadata for network transmission
    // Format: <count>;<exam1_meta>;<exam2_meta>;...
    std::string serializeForNetwork() const;

    // Get exam count
    size_t count() const { return exams.size(); }

    // Clear all exams
    void clear() { exams.clear(); }
};

} // namespace server

#endif // SERVER_MODEL_EXAM_H
