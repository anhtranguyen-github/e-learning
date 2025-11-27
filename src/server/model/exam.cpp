#include "server/model/exam.h"
#include <sstream>

namespace server {

// ============================================================================
// Exam Implementation
// ============================================================================

std::string Exam::serializeForNetwork(ExamType type) const {
    std::ostringstream oss;
    
    switch (type) {
        case ExamType::QUESTIONS:
            oss << "QUESTIONS|";
            for (size_t i = 0; i < questions.size(); ++i) {
                if (i > 0) oss << ",";
                oss << questions[i];
            }
            break;
            
        case ExamType::FULL:
            oss << "FULL|";
            oss << "ID:" << examId << "|";
            oss << "LESSON_ID:" << lessonId << "|";
            oss << "TITLE:" << title << "|";
            oss << "TYPE:" << this->type << "|";
            oss << "LEVEL:" << level << "|";
            oss << "QUESTIONS:";
            for (size_t i = 0; i < questions.size(); ++i) {
                if (i > 0) oss << ",";
                oss << questions[i];
            }
            break;
    }
    
    return oss.str();
}

std::string Exam::serializeMetadata() const {
    std::ostringstream oss;
    oss << examId << "|" << lessonId << "|" << title << "|" << type << "|" << level;
    return oss.str();
}

Payloads::ExamDTO Exam::toDTO() const {
    Payloads::ExamDTO dto;
    dto.id = std::to_string(examId);
    dto.lessonId = std::to_string(lessonId);
    dto.title = title;
    dto.type = type;
    dto.level = level;
    dto.questions = questions;
    return dto;
}

Payloads::ExamMetadataDTO Exam::toMetadataDTO() const {
    Payloads::ExamMetadataDTO dto;
    dto.id = std::to_string(examId);
    dto.lessonId = std::to_string(lessonId);
    dto.title = title;
    dto.type = type;
    dto.level = level;
    return dto;
}

// ============================================================================
// ExamList Implementation
// ============================================================================

ExamList ExamList::filter(const std::string& type, const std::string& level, int lessonId) const {
    ExamList filtered;
    
    for (const auto& exam : exams) {
        bool typeMatch = type.empty() || exam.getType() == type;
        bool levelMatch = level.empty() || exam.getLevel() == level;
        bool lessonIdMatch = (lessonId == -1) || (exam.getLessonId() == lessonId);
        
        if (typeMatch && levelMatch && lessonIdMatch) {
            filtered.addExam(exam);
        }
    }
    
    return filtered;
}

std::string ExamList::serializeForNetwork() const {
    std::ostringstream oss;
    oss << exams.size();
    
    for (const auto& exam : exams) {
        oss << ";" << exam.serializeMetadata();
    }
    
    return oss.str();
}

} // namespace server
