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
            oss << "QUESTION|" << (questions.empty() ? "" : questions[0].getText());
            break;
            
        case ExerciseType::OPTIONS:
            oss << "OPTIONS|";
            if (!questions.empty()) {
                for (size_t i = 0; i < questions[0].getOptions().size(); ++i) {
                    if (i > 0) oss << ",";
                    oss << questions[0].getOptions()[i];
                }
            }
            break;
            
        case ExerciseType::ANSWER:
            oss << "ANSWER|" << (questions.empty() ? "" : questions[0].getAnswer());
            break;
            
        case ExerciseType::EXPLANATION:
            oss << "EXPLANATION|" << (questions.empty() ? "" : questions[0].getExplanation());
            break;
            
        case ExerciseType::FULL:
        default:
            // Format: id;lesson_id;title;type;level;question_count;question1_json^question2_json...
            std::stringstream ss;
            ss << exerciseId << ";" << lessonId << ";" << title << ";" << this->type << ";" << level << ";";
            
            ss << questions.size() << ";";
            for (size_t i = 0; i < questions.size(); ++i) {
                ss << questions[i].toJsonString();
                if (i < questions.size() - 1) ss << "^";
            }
            
            return ss.str();
    }
    
    return oss.str();
}

std::string Exercise::serializeMetadata() const {
    return std::to_string(exerciseId) + ";" + std::to_string(lessonId) + ";" + title + ";" + type + ";" + level;
}

Payloads::ExerciseDTO Exercise::toDTO() const {
    Payloads::ExerciseDTO dto;
    dto.id = std::to_string(exerciseId);
    dto.lessonId = std::to_string(lessonId);
    dto.title = title;
    dto.type = type;
    dto.level = level;
    
    for (const auto& q : questions) {
        dto.questions.push_back(q.toJsonString());
    }
    
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
