#ifndef COMMON_PAYLOADS_H
#define COMMON_PAYLOADS_H

#include "common/utils.h"
#include <string>
#include <vector>
#include <stdexcept>

namespace Payloads {

    // Base Interface
    struct ISerializable {
        virtual std::string serialize() const = 0;
        virtual void deserialize(const std::string& raw) = 0;
        virtual ~ISerializable() = default;
    };

    // LoginRequest
    struct LoginRequest : public ISerializable {
        std::string username;
        std::string password;

        std::string serialize() const override {
            std::vector<std::string> parts = {username, password};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) username = parts[0];
            if (parts.size() >= 2) password = parts[1];
        }
    };

    // LessonListRequest
    struct LessonListRequest : public ISerializable {
        std::string sessionToken;
        std::string topic;
        std::string level;

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, topic, level};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) topic = parts[1];
            if (parts.size() >= 3) level = parts[2];
        }
    };

    // SubmitAnswerRequest
    struct SubmitAnswerRequest : public ISerializable {
        std::string sessionToken;
        std::string targetType;
        std::string targetId;
        std::string answer;

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, targetType, targetId, answer};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) targetType = parts[1];
            if (parts.size() >= 3) targetId = parts[2];
            if (parts.size() >= 4) answer = parts[3];
        }
    };

    // StudyLessonRequest
    struct StudyLessonRequest : public ISerializable {
        std::string sessionToken;
        std::string lessonId;
        std::string lessonType;

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, lessonId, lessonType};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) lessonId = parts[1];
            if (parts.size() >= 3) lessonType = parts[2];
        }
    };

    // PrivateMessageRequest
    struct PrivateMessageRequest : public ISerializable {
        std::string sessionToken;
        std::string recipient;
        std::string message;

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, recipient, message};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) recipient = parts[1];
            if (parts.size() >= 3) message = parts[2];
        }
    };

    // ChatHistoryRequest
    struct ChatHistoryRequest : public ISerializable {
        std::string sessionToken;
        std::string otherUser;

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, otherUser};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) otherUser = parts[1];
        }
    };

    // ExerciseListRequest
    struct ExerciseListRequest : public ISerializable {
        std::string sessionToken;
        std::string type;
        std::string level;
        std::string lessonId;

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, type, level, lessonId};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) type = parts[1];
            if (parts.size() >= 3) level = parts[2];
            if (parts.size() >= 4) lessonId = parts[3];
        }
    };

    // StudyExerciseRequest
    struct StudyExerciseRequest : public ISerializable {
        std::string sessionToken;
        std::string exerciseId;
        std::string exerciseType;

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, exerciseId, exerciseType};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) exerciseId = parts[1];
            if (parts.size() >= 3) exerciseType = parts[2];
        }
    };

    // SpecificExerciseRequest
    struct SpecificExerciseRequest : public ISerializable {
        std::string sessionToken;
        std::string exerciseId;

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, exerciseId};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) exerciseId = parts[1];
        }
    };

    // ResultRequest
    struct ResultRequest : public ISerializable {
        std::string sessionToken;
        std::string targetType;
        std::string targetId;

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, targetType, targetId};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) targetType = parts[1];
            if (parts.size() >= 3) targetId = parts[2];
        }
    };

    // ResultListRequest
    struct ResultListRequest : public ISerializable {
        std::string sessionToken;
        std::string targetType;

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, targetType};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) targetType = parts[1];
        }
    };

    // ExamListRequest
    struct ExamListRequest : public ISerializable {
        std::string sessionToken;
        std::string type;
        std::string level;
        std::string lessonId;

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, type, level, lessonId};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) type = parts[1];
            if (parts.size() >= 3) level = parts[2];
            if (parts.size() >= 4) lessonId = parts[3];
        }
    };

    struct ExamRequest : public ISerializable {
        std::string sessionToken;
        std::string examId;

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, examId};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) examId = parts[1];
        }
    };

    // GenericResponse
    struct GenericResponse : public ISerializable {
        bool success;
        std::string message;

        std::string serialize() const override {
            std::vector<std::string> parts = {success ? "1" : "0", message};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) success = (parts[0] == "1");
            if (parts.size() >= 2) message = parts[1];
        }
    };

    struct LessonMetadataDTO : public ISerializable {
        std::string id;
        std::string title;
        std::string topic;
        std::string level;

        std::string serialize() const override {
            std::vector<std::string> parts = {id, title, topic, level};
            return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) id = parts[0];
            if (parts.size() >= 2) title = parts[1];
            if (parts.size() >= 3) topic = parts[2];
            if (parts.size() >= 4) level = parts[3];
        }
    };

    struct LessonDTO : public ISerializable {
        std::string id;
        std::string title;
        std::string topic;
        std::string level;
        std::string videoUrl;
        std::string audioUrl;
        std::string textContent;
        std::vector<std::string> vocabulary;
        std::vector<std::string> grammar;

        std::string serialize() const override {
            std::vector<std::string> parts;
            parts.push_back(id);
            parts.push_back(title);
            parts.push_back(topic);
            parts.push_back(level);
            parts.push_back(videoUrl);
            parts.push_back(audioUrl);
            parts.push_back(textContent);
            parts.push_back(utils::join(vocabulary, ','));
            parts.push_back(utils::join(grammar, ','));
            return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) id = parts[0];
            if (parts.size() >= 2) title = parts[1];
            if (parts.size() >= 3) topic = parts[2];
            if (parts.size() >= 4) level = parts[3];
            if (parts.size() >= 5) videoUrl = parts[4];
            if (parts.size() >= 6) audioUrl = parts[5];
            if (parts.size() >= 7) textContent = parts[6];
            if (parts.size() >= 8) vocabulary = utils::split(parts[7], ',');
            if (parts.size() >= 9) grammar = utils::split(parts[8], ',');
        }
    };

    struct ExerciseMetadataDTO : public ISerializable {
        std::string id;
        std::string lessonId;
        std::string title;
        std::string type;
        std::string level;

        std::string serialize() const override {
            std::vector<std::string> parts = {id, lessonId, title, type, level};
            return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) id = parts[0];
            if (parts.size() >= 2) lessonId = parts[1];
            if (parts.size() >= 3) title = parts[2];
            if (parts.size() >= 4) type = parts[3];
            if (parts.size() >= 5) level = parts[4];
        }
    };

    struct ExerciseDTO : public ISerializable {
        std::string id;
        std::string lessonId;
        std::string title;
        std::string type;
        std::string level;
        std::vector<std::string> questions;

        std::string serialize() const override {
            std::vector<std::string> parts;
            parts.push_back(id);
            parts.push_back(lessonId);
            parts.push_back(title);
            parts.push_back(type);
            parts.push_back(level);
            parts.push_back(utils::join(questions, '^'));
            return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) id = parts[0];
            if (parts.size() >= 2) lessonId = parts[1];
            if (parts.size() >= 3) title = parts[2];
            if (parts.size() >= 4) type = parts[3];
            if (parts.size() >= 5) level = parts[4];
            if (parts.size() >= 6) questions = utils::split(parts[5], '^');
        }
    };

    struct ExamMetadataDTO : public ISerializable {
        std::string id;
        std::string lessonId;
        std::string title;
        std::string type;
        std::string level;

        std::string serialize() const override {
            std::vector<std::string> parts = {id, lessonId, title, type, level};
            return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) id = parts[0];
            if (parts.size() >= 2) lessonId = parts[1];
            if (parts.size() >= 3) title = parts[2];
            if (parts.size() >= 4) type = parts[3];
            if (parts.size() >= 5) level = parts[4];
        }
    };

    struct ExamDTO : public ISerializable {
        std::string id;
        std::string lessonId;
        std::string title;
        std::string type;
        std::string level;
        std::vector<std::string> questions;

        std::string serialize() const override {
            std::vector<std::string> parts;
            parts.push_back(id);
            parts.push_back(lessonId);
            parts.push_back(title);
            parts.push_back(type);
            parts.push_back(level);
            parts.push_back(utils::join(questions, '^'));
            return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) id = parts[0];
            if (parts.size() >= 2) lessonId = parts[1];
            if (parts.size() >= 3) title = parts[2];
            if (parts.size() >= 4) type = parts[3];
            if (parts.size() >= 5) level = parts[4];
            if (parts.size() >= 6) questions = utils::split(parts[5], '^');
        }
    };

    struct ResultDTO : public ISerializable {
        std::string score;
        std::string feedback;

        std::string serialize() const override {
            std::vector<std::string> parts = {score, feedback};
            return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) score = parts[0];
            if (parts.size() >= 2) feedback = parts[1];
        }
    };

    struct ResultSummaryDTO : public ISerializable {
        std::string targetId;
        std::string score;
        std::string status;
        std::string feedback;

        std::string serialize() const override {
            std::vector<std::string> parts = {targetId, score, status, feedback};
            return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) targetId = parts[0];
            if (parts.size() >= 2) score = parts[1];
            if (parts.size() >= 3) status = parts[2];
            if (parts.size() >= 4) feedback = parts[3];
        }
    };

    struct PendingSubmissionDTO : public ISerializable {
        std::string resultId;
        std::string userName;
        std::string targetType;
        std::string targetTitle;
        std::string submittedAt;
        std::string userAnswer;

        std::string serialize() const override {
            std::vector<std::string> parts = {resultId, userName, targetType, targetTitle, submittedAt, userAnswer};
            return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) resultId = parts[0];
            if (parts.size() >= 2) userName = parts[1];
            if (parts.size() >= 3) targetType = parts[2];
            if (parts.size() >= 4) targetTitle = parts[3];
            if (parts.size() >= 5) submittedAt = parts[4];
            if (parts.size() >= 6) userAnswer = parts[5];
        }
    };

    struct PendingSubmissionsRequest : public ISerializable {
        std::string sessionToken;

        std::string serialize() const override {
            return sessionToken;
        }

        void deserialize(const std::string& raw) override {
            sessionToken = raw;
        }
    };

    struct GradeSubmissionRequest : public ISerializable {
        std::string sessionToken;
        std::string resultId;
        std::string score;
        std::string feedback;

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, resultId, score, feedback};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) resultId = parts[1];
            if (parts.size() >= 3) score = parts[2];
            if (parts.size() >= 4) feedback = parts[3];
        }
    };

} // namespace Payloads

#endif // COMMON_PAYLOADS_H
