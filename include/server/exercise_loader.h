#ifndef EXERCISE_LOADER_H
#define EXERCISE_LOADER_H

#include "server/database.h"
#include <string>
#include <vector>
#include <memory>
#include <json/json.h>

namespace server {

// Enum for exercise types
enum class ExerciseType {
    REWRITE_SENTENCE,
    ESSAY,
    SPEAKING
};

// Enum for exam types
enum class ExamType {
    MULTIPLE_CHOICE,
    FILL_BLANK,
    ORDER_SENTENCE
};

// Represents a single exercise
class Exercise {
private:
    int exerciseId;
    int lessonId;
    ExerciseType type;
    std::string question;
    std::string answer;
    std::string mediaUrl;
    int createdBy;

public:
    Exercise() : exerciseId(-1), lessonId(-1), createdBy(-1) {}

    // Getters
    int getExerciseId() const { return exerciseId; }
    int getLessonId() const { return lessonId; }
    ExerciseType getType() const { return type; }
    std::string getQuestion() const { return question; }
    std::string getAnswer() const { return answer; }
    std::string getMediaUrl() const { return mediaUrl; }
    int getCreatedBy() const { return createdBy; }

    // Setters
    void setExerciseId(int id) { exerciseId = id; }
    void setLessonId(int id) { lessonId = id; }
    void setType(ExerciseType t) { type = t; }
    void setQuestion(const std::string& q) { question = q; }
    void setAnswer(const std::string& a) { answer = a; }
    void setMediaUrl(const std::string& url) { mediaUrl = url; }
    void setCreatedBy(int id) { createdBy = id; }

    std::string serializeForNetwork() const;
};

// Represents a single exam
class Exam {
private:
    int examId;
    int lessonId;
    ExamType type;
    Json::Value question;
    int createdBy;

public:
    Exam() : examId(-1), lessonId(-1), createdBy(-1) {}

    // Getters
    int getExamId() const { return examId; }
    int getLessonId() const { return lessonId; }
    ExamType getType() const { return type; }
    Json::Value getQuestion() const { return question; }
    int getCreatedBy() const { return createdBy; }

    // Setters
    void setExamId(int id) { examId = id; }
    void setLessonId(int id) { lessonId = id; }
    void setType(ExamType t) { type = t; }
    void setQuestion(const Json::Value& q) { question = q; }
    void setCreatedBy(int id) { createdBy = id; }

    std::string serializeForNetwork() const;
};

// Loader class for loading exercises and exams from the database
class ExerciseLoader {
private:
    std::shared_ptr<Database> db;

    ExerciseType stringToExerciseType(const std::string& typeStr) const;
    ExamType stringToExamType(const std::string& typeStr) const;

public:
    ExerciseLoader(std::shared_ptr<Database> database);

    // Load a specific exercise by ID
    Exercise loadExerciseById(int exerciseId);

    // Load a specific exam by ID
    Exam loadExamById(int examId);
};

} // namespace server

#endif // EXERCISE_LOADER_H