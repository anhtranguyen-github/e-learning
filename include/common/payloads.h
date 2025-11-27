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

} // namespace Payloads

#endif // COMMON_PAYLOADS_H
