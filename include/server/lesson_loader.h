#ifndef LESSON_LOADER_H
#define LESSON_LOADER_H

#include "server/database.h"
#include <string>
#include <vector>
#include <memory>
#include <sstream>

namespace server {

// Lesson content types for selective loading
enum class LessonType {
    VIDEO,
    AUDIO,
    TEXT,
    VOCABULARY,
    GRAMMAR,
    FULL  // All content
};

// Represents a single lesson with all its content
class Lesson {
private:
    int lessonId;
    std::string title;
    std::string topic;
    std::string level;
    std::string videoUrl;
    std::string audioUrl;
    std::string textContent;
    std::vector<std::string> vocabulary;
    std::vector<std::string> grammar;

public:
    Lesson() : lessonId(-1) {}
    
    Lesson(int id, const std::string& t, const std::string& top, const std::string& lv)
        : lessonId(id), title(t), topic(top), level(lv) {}

    // Getters
    int getLessonId() const { return lessonId; }
    std::string getTitle() const { return title; }
    std::string getTopic() const { return topic; }
    std::string getLevel() const { return level; }
    std::string getVideoUrl() const { return videoUrl; }
    std::string getAudioUrl() const { return audioUrl; }
    std::string getTextContent() const { return textContent; }
    std::vector<std::string> getVocabulary() const { return vocabulary; }
    std::vector<std::string> getGrammar() const { return grammar; }

    // Setters
    void setLessonId(int id) { lessonId = id; }
    void setTitle(const std::string& t) { title = t; }
    void setTopic(const std::string& t) { topic = t; }
    void setLevel(const std::string& l) { level = l; }
    void setVideoUrl(const std::string& url) { videoUrl = url; }
    void setAudioUrl(const std::string& url) { audioUrl = url; }
    void setTextContent(const std::string& text) { textContent = text; }
    void setVocabulary(const std::vector<std::string>& vocab) { vocabulary = vocab; }
    void setGrammar(const std::vector<std::string>& gram) { grammar = gram; }

    // Serialize only the requested content type for network transmission
    // Returns formatted string containing only the requested field(s)
    std::string serializeForNetwork(LessonType type) const;

    // Serialize metadata (id, title, topic, level) for lesson list
    std::string serializeMetadata() const;
};

// Container for multiple lessons with filtering capabilities
class LessonList {
private:
    std::vector<Lesson> lessons;

public:
    LessonList() = default;

    // Add a lesson to the list
    void addLesson(const Lesson& lesson) {
        lessons.push_back(lesson);
    }

    // Get all lessons
    const std::vector<Lesson>& getLessons() const { return lessons; }

    // Filter lessons by topic and/or level
    // Empty string means no filter for that field
    LessonList filter(const std::string& topic, const std::string& level) const;

    // Serialize all lessons metadata for network transmission
    // Format: <count>;<lesson1_meta>;<lesson2_meta>;...
    std::string serializeForNetwork() const;

    // Get lesson count
    size_t count() const { return lessons.size(); }

    // Clear all lessons
    void clear() { lessons.clear(); }
};

// Loader class for loading lessons from database
class LessonLoader {
private:
    std::shared_ptr<Database> db;
    bool parseLessonFromRow(PGresult* result, int row, Lesson& lesson) const;
    // Helper to parse JSON array from PostgreSQL JSONB field
    std::vector<std::string> parseJsonArray(const std::string& jsonStr) const;

public:
    LessonLoader(std::shared_ptr<Database> database);

    // Load all lessons from database
    LessonList loadAllLessons();

    // Load a specific lesson by ID with full content
    Lesson loadLessonById(int lessonId);

    // Load lessons filtered by topic and/or level
    LessonList loadLessonsByFilter(const std::string& topic, const std::string& level);
};

} // namespace server

#endif // LESSON_LOADER_H
