#ifndef LESSON_REPOSITORY_H
#define LESSON_REPOSITORY_H

#include "server/database.h"
#include "server/model/lesson.h"
#include "common/payloads.h"
#include <string>
#include <vector>
#include <memory>
#include <sstream>

namespace server {

// Repository class for accessing lessons from database
class LessonRepository {
private:
    std::shared_ptr<Database> db;
    bool parseLessonFromRow(PGresult* result, int row, Lesson& lesson) const;
    // Helper to parse JSON array from PostgreSQL JSONB field
    std::vector<std::string> parseJsonArray(const std::string& jsonStr) const;

public:
    LessonRepository(std::shared_ptr<Database> database);

    // Load all lessons from database
    LessonList loadAllLessons();

    // Load a specific lesson by ID with full content
    Lesson loadLessonById(int lessonId);

    // Load lessons filtered by topic and/or level
    LessonList loadLessonsByFilter(const std::string& topic, const std::string& level);
};

} // namespace server

#endif // LESSON_REPOSITORY_H
