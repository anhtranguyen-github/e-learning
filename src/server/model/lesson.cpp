#include "server/model/lesson.h"
#include <sstream>

namespace server {

// ============================================================================
// Lesson Implementation
// ============================================================================

std::string Lesson::serializeForNetwork(LessonType type) const {
    switch (type) {
        case LessonType::VIDEO:
            return videoUrl;
        case LessonType::AUDIO:
            return audioUrl;
        case LessonType::TEXT:
            return textContent;
        case LessonType::VOCABULARY: {
            std::string result;
            for (size_t i = 0; i < vocabulary.size(); ++i) {
                result += vocabulary[i];
                if (i < vocabulary.size() - 1) result += "|";
            }
            return result;
        }
        case LessonType::GRAMMAR: {
            std::string result;
            for (size_t i = 0; i < grammar.size(); ++i) {
                result += grammar[i];
                if (i < grammar.size() - 1) result += "|";
            }
            return result;
        }
        case LessonType::FULL:
        default:
            // Format: id;title;topic;level;video;audio;text;vocab_count;vocab1|vocab2...;grammar_count;gram1|gram2...
            std::stringstream ss;
            ss << lessonId << ";" << title << ";" << topic << ";" << level << ";"
               << videoUrl << ";" << audioUrl << ";" << textContent << ";";
            
            ss << vocabulary.size() << ";";
            for (size_t i = 0; i < vocabulary.size(); ++i) {
                ss << vocabulary[i];
                if (i < vocabulary.size() - 1) ss << "|";
            }
            ss << ";";
            
            ss << grammar.size() << ";";
            for (size_t i = 0; i < grammar.size(); ++i) {
                ss << grammar[i];
                if (i < grammar.size() - 1) ss << "|";
            }
            
            return ss.str();
    }
}

std::string Lesson::serializeMetadata() const {
    return std::to_string(lessonId) + ";" + title + ";" + topic + ";" + level;
}

Payloads::LessonDTO Lesson::toDTO() const {
    Payloads::LessonDTO dto;
    dto.id = std::to_string(lessonId);
    dto.title = title;
    dto.topic = topic;
    dto.level = level;
    dto.videoUrl = videoUrl;
    dto.audioUrl = audioUrl;
    dto.textContent = textContent;
    dto.vocabulary = vocabulary;
    dto.grammar = grammar;
    return dto;
}

Payloads::LessonMetadataDTO Lesson::toMetadataDTO() const {
    Payloads::LessonMetadataDTO dto;
    dto.id = std::to_string(lessonId);
    dto.title = title;
    dto.topic = topic;
    dto.level = level;
    return dto;
}

// ============================================================================
// LessonList Implementation
// ============================================================================

LessonList LessonList::filter(const std::string& topic, const std::string& level) const {
    LessonList filteredList;
    for (const auto& lesson : lessons) {
        bool topicMatch = topic.empty() || lesson.getTopic() == topic;
        bool levelMatch = level.empty() || lesson.getLevel() == level;
        
        if (topicMatch && levelMatch) {
            filteredList.addLesson(lesson);
        }
    }
    return filteredList;
}

std::string LessonList::serializeForNetwork() const {
    std::stringstream ss;
    ss << lessons.size();
    
    for (const auto& lesson : lessons) {
        ss << ";" << lesson.serializeMetadata();
    }
    
    return ss.str();
}

} // namespace server
