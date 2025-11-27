#include "server/model/exercise.h"
#include "common/utils.h"
#include <sstream>

namespace server {

// ============================================================================
// Exercise Implementation
// ============================================================================

std::string Exercise::serializeForNetwork(ExerciseType type) const {
    std::ostringstream oss;
    
    switch (type) {
        case ExerciseType::QUESTION:
            oss << "QUESTION|" << question;
            break;
            
        case ExerciseType::OPTIONS:
            oss << "OPTIONS|";
            for (size_t i = 0; i < options.size(); ++i) {
                if (i > 0) oss << ",";
                oss << options[i];
            }
            break;
            
        case ExerciseType::ANSWER:
            oss << "ANSWER|" << answer;
            break;
            
        case ExerciseType::EXPLANATION:
            oss << "EXPLANATION|" << explanation;
            break;
            
        case ExerciseType::FULL:
            oss << "FULL|";
            oss << "ID:" << exerciseId << "|";
            oss << "LESSON_ID:" << lessonId << "|";
            oss << "TITLE:" << title << "|";
            oss << "TYPE:" << this->type << "|"; // Use this->type (string member) instead of enum parameter
            oss << "LEVEL:" << level << "|";
            oss << "QUESTION:" << question << "|";
            oss << "OPTIONS:";
            for (size_t i = 0; i < options.size(); ++i) {
                if (i > 0) oss << ",";
                oss << options[i];
            }
            oss << "|ANSWER:" << answer << "|";
            oss << "EXPLANATION:" << explanation;
            break;
    }
    
    return oss.str();
}

std::string Exercise::serializeMetadata() const {
    std::ostringstream oss;
    oss << exerciseId << "|" << lessonId << "|" << title << "|" << type << "|" << level;
    return oss.str();
}

Payloads::ExerciseDTO Exercise::toDTO() const {
    Payloads::ExerciseDTO dto;
    dto.id = std::to_string(exerciseId);
    dto.lessonId = std::to_string(lessonId);
    dto.title = title;
    dto.type = type;
    dto.level = level;
    dto.question = question;
    dto.options = options;
    dto.answer = answer;
    dto.explanation = explanation;
    return dto;
}

Payloads::ExerciseMetadataDTO Exercise::toMetadataDTO() const {
    Payloads::ExerciseMetadataDTO dto;
    dto.id = std::to_string(exerciseId);
    dto.lessonId = std::to_string(lessonId);
    dto.title = title;
    dto.type = type;
    dto.level = level;
    return dto;
}

// ============================================================================
// ExerciseList Implementation
// ============================================================================

ExerciseList ExerciseList::filter(const std::string& type, const std::string& level, int lessonId) const {
    ExerciseList filtered;
    
    for (const auto& exercise : exercises) {
        bool typeMatch = type.empty() || exercise.getType() == type;
        bool levelMatch = level.empty() || exercise.getLevel() == level;
        bool lessonIdMatch = (lessonId == -1) || (exercise.getLessonId() == lessonId);
        
        if (typeMatch && levelMatch && lessonIdMatch) {
            filtered.addExercise(exercise);
        }
    }
    
    return filtered;
}

std::string ExerciseList::serializeForNetwork() const {
    std::ostringstream oss;
    oss << exercises.size();
    
    for (const auto& exercise : exercises) {
        oss << ";" << exercise.serializeMetadata();
    }
    
    return oss.str();
}

} // namespace server
