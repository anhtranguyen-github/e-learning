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
        std::string messageType; // "TEXT" or "AUDIO"
        std::string content;

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, recipient, messageType, content};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) recipient = parts[1];
            if (parts.size() >= 3) messageType = parts[2];
            if (parts.size() >= 4) content = parts[3];
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
        std::string targetType; // Added targetType
        std::string title; // Added title

        std::string serialize() const override {
            std::vector<std::string> parts = {targetId, score, status, feedback, targetType, title};
            return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) targetId = parts[0];
            if (parts.size() >= 2) score = parts[1];
            if (parts.size() >= 3) status = parts[2];
            if (parts.size() >= 4) feedback = parts[3];
            if (parts.size() >= 5) targetType = parts[4];
            if (parts.size() >= 6) title = parts[5];
        }
    };

    struct PendingSubmissionDTO : public ISerializable {
        std::string resultId;
        std::string userName;
        std::string targetType;
        std::string targetTitle;
        std::string submittedAt;
        std::string userAnswer;
        std::string targetId;
        std::string status; // "pending" or "graded"

        std::string serialize() const override {
            std::vector<std::string> parts = {resultId, userName, targetType, targetTitle, submittedAt, userAnswer, targetId, status};
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
            if (parts.size() >= 7) targetId = parts[6];
            if (parts.size() >= 8) status = parts[7];
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
        std::string userId;
        std::string score;
        std::string feedback;

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, resultId, userId, score, feedback};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) resultId = parts[1];
            if (parts.size() >= 3) userId = parts[2];
            if (parts.size() >= 4) score = parts[3];
            if (parts.size() >= 5) feedback = parts[4];
        }
    };

    // SubmissionDTO - represents a student submission for teacher review
    struct SubmissionDTO : public ISerializable {
        std::string resultId;
        std::string studentName;
        std::string studentId;
        std::string targetType;     // "exercise" or "exam"
        std::string targetTitle;
        std::string targetId;
        std::string lessonId;
        std::string submittedAt;
        std::string status;         // "pending" or "graded"
        std::string score;
        std::string userAnswer;

        std::string serialize() const override {
            std::vector<std::string> parts = {resultId, studentName, targetType, targetTitle, targetId, submittedAt, status, score, userAnswer, studentId, lessonId};
            return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) resultId = parts[0];
            if (parts.size() >= 2) studentName = parts[1];
            if (parts.size() >= 3) targetType = parts[2];
            if (parts.size() >= 4) targetTitle = parts[3];
            if (parts.size() >= 5) targetId = parts[4];
            if (parts.size() >= 6) submittedAt = parts[5];
            if (parts.size() >= 7) status = parts[6];
            if (parts.size() >= 8) score = parts[7];
            if (parts.size() >= 9) userAnswer = parts[8];
            if (parts.size() >= 10) studentId = parts[9];
            if (parts.size() >= 11) lessonId = parts[10];
        }
    };

    // FeedbackDTO - teacher feedback for a submission
    struct FeedbackDTO : public ISerializable {
        std::string resultId;
        std::string feedbackText;
        std::string feedbackType;   // "text" or "audio"
        std::string audioData;      // Base64 encoded audio if type is "audio"
        std::string gradedBy;
        std::string gradedAt;

        std::string serialize() const override {
            std::vector<std::string> parts = {resultId, feedbackText, feedbackType, audioData, gradedBy, gradedAt};
            return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) resultId = parts[0];
            if (parts.size() >= 2) feedbackText = parts[1];
            if (parts.size() >= 3) feedbackType = parts[2];
            if (parts.size() >= 4) audioData = parts[3];
            if (parts.size() >= 5) gradedBy = parts[4];
            if (parts.size() >= 6) gradedAt = parts[5];
        }
    };

    // AddFeedbackRequest - request to add feedback to a submission
    struct AddFeedbackRequest : public ISerializable {
        std::string sessionToken;
        std::string resultId;
        std::string feedbackText;
        std::string feedbackType;   // "text" or "audio"
        std::string audioData;      // Base64 if audio

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, resultId, feedbackText, feedbackType, audioData};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) resultId = parts[1];
            if (parts.size() >= 3) feedbackText = parts[2];
            if (parts.size() >= 4) feedbackType = parts[3];
            if (parts.size() >= 5) audioData = parts[4];
        }
    };

    struct ResultDetailRequest : public ISerializable {
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

    struct QuestionResultDTO : public ISerializable {
        std::string questionText;
        std::string userAnswer;
        std::string correctAnswer;
        std::string status;
        std::string score;
        std::string comment;

        std::string serialize() const override {
            std::vector<std::string> parts = {questionText, userAnswer, correctAnswer, status, score, comment};
            return utils::join(parts, '^');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '^');
            if (parts.size() >= 1) questionText = parts[0];
            if (parts.size() >= 2) userAnswer = parts[1];
            if (parts.size() >= 3) correctAnswer = parts[2];
            if (parts.size() >= 4) status = parts[3];
            if (parts.size() >= 5) score = parts[4];
            if (parts.size() >= 6) comment = parts[5];
        }
    };

    struct ResultAttemptDTO : public ISerializable {
        std::string resultId;
        std::string score;
        std::string feedback;

        std::string serialize() const override {
            std::vector<std::string> parts = {resultId, score, feedback};
            return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) resultId = parts[0];
            if (parts.size() >= 2) score = parts[1];
            if (parts.size() >= 3) feedback = parts[2];
        }
    };

    struct ResultDetailDTO : public ISerializable {
        std::string targetId;
        std::string targetType;
        std::string title;
        std::string score;
        std::string feedback;
        std::vector<QuestionResultDTO> questions;
        std::vector<ResultAttemptDTO> attempts;

        std::string serialize() const override {
            std::string base = utils::join({targetId, targetType, title}, '|');
            std::vector<std::string> qParts;
            for (const auto& q : questions) {
                qParts.push_back(q.serialize());
            }
            std::string questionsStr = utils::join(qParts, '~');

            std::vector<std::string> attemptParts;
            if (!attempts.empty()) {
                std::string firstAttempt = attempts[0].serialize();
                firstAttempt += "|" + questionsStr;
                attemptParts.push_back(firstAttempt);

                for (size_t i = 1; i < attempts.size(); ++i) {
                    attemptParts.push_back(attempts[i].serialize());
                }
            }

            if (!attemptParts.empty()) {
                base += "|" + utils::join(attemptParts, ';');
            }

            return base;
        }

        void deserialize(const std::string& raw) override {
            auto chunks = utils::split(raw, ';');
            if (chunks.empty()) return;

            auto headerParts = utils::split(chunks[0], '|');
            if (headerParts.size() >= 1) targetId = headerParts[0];
            if (headerParts.size() >= 2) targetType = headerParts[1];
            if (headerParts.size() >= 3) title = headerParts[2];
            if (headerParts.size() >= 4) {
                ResultAttemptDTO firstAttempt;
                firstAttempt.resultId = headerParts[3];
                if (headerParts.size() >= 5) firstAttempt.score = headerParts[4];
                if (headerParts.size() >= 6) firstAttempt.feedback = headerParts[5];
                attempts.push_back(firstAttempt);
                score = firstAttempt.score;
                feedback = firstAttempt.feedback;
            }

            if (headerParts.size() >= 7 && !headerParts[6].empty()) {
                auto qList = utils::split(headerParts[6], '~');
                for (const auto& qRaw : qList) {
                    QuestionResultDTO q;
                    q.deserialize(qRaw);
                    questions.push_back(q);
                }
            }

            for (size_t i = 1; i < chunks.size(); ++i) {
                if (chunks[i].empty()) continue;
                auto attemptParts = utils::split(chunks[i], '|');
                if (attemptParts.size() >= 1) {
                    ResultAttemptDTO attempt;
                    attempt.resultId = attemptParts[0];
                    if (attemptParts.size() >= 2) attempt.score = attemptParts[1];
                    if (attemptParts.size() >= 3) attempt.feedback = attemptParts[2];
                    attempts.push_back(attempt);
                }
            }
        }
    };

    struct ChatMessageDTO : public ISerializable {
        std::string sender;
        std::string messageType;
        std::string content;
        std::string timestamp;

        std::string serialize() const override {
            std::vector<std::string> parts = {sender, messageType, content, timestamp};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sender = parts[0];
            if (parts.size() >= 2) messageType = parts[1];
            if (parts.size() >= 3) content = parts[2];
            if (parts.size() >= 4) timestamp = parts[3];
        }
    };

    struct ChatHistoryDTO {
        std::vector<ChatMessageDTO> messages;

        std::string serialize() const {
            std::string result;
            for (size_t i = 0; i < messages.size(); ++i) {
                result += messages[i].serialize();
                if (i < messages.size() - 1) result += "|";
            }
            return result;
        }

        void deserialize(const std::string& data) {
            messages.clear();
            std::stringstream ss(data);
            std::string segment;
            while (std::getline(ss, segment, '|')) {
                ChatMessageDTO msg;
                msg.deserialize(segment);
                messages.push_back(msg);
            }
        }
    };

    struct RecentChatsRequest {
        std::string sessionToken;

        std::string serialize() const {
            return sessionToken;
        }

        void deserialize(const std::string& data) {
            sessionToken = data;
        }
    };

    struct RecentChatDTO {
        int userId;
        std::string username;
        std::string lastMessage;
        std::string timestamp;

        std::string serialize() const {
            return std::to_string(userId) + ";" + username + ";" + lastMessage + ";" + timestamp;
        }

        void deserialize(const std::string& data) {
            std::stringstream ss(data);
            std::string segment;
            std::getline(ss, segment, ';'); userId = std::stoi(segment);
            std::getline(ss, username, ';');
            std::getline(ss, lastMessage, ';');
            std::getline(ss, timestamp, ';');
        }
    };

    struct RecentChatsDTO {
        std::vector<RecentChatDTO> chats;

        std::string serialize() const {
            std::string result;
            for (size_t i = 0; i < chats.size(); ++i) {
                result += chats[i].serialize();
                if (i < chats.size() - 1) result += "|";
            }
            return result;
        }

        void deserialize(const std::string& data) {
            chats.clear();
            std::stringstream ss(data);
            std::string segment;
            while (std::getline(ss, segment, '|')) {
                RecentChatDTO chat;
                chat.deserialize(segment);
                chats.push_back(chat);
            }
        }
    };

    // --- Game Payloads ---

    // GameListRequest
    struct GameListRequest : public ISerializable {
        std::string sessionToken;

        std::string serialize() const override {
            return sessionToken;
        }

        void deserialize(const std::string& raw) override {
            sessionToken = raw;
        }
    };

    struct GameMetadataDTO : public ISerializable {
        std::string type; // "sentence_match", "picture_match", etc
        std::string description;

        std::string serialize() const override {
             std::vector<std::string> parts = {type, description};
             return utils::join(parts, '|');
        }

       void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) type = parts[0];
            if (parts.size() >= 2) description = parts[1];
        }
    };

    // GameLevelListRequest
    struct GameLevelListRequest : public ISerializable {
        std::string sessionToken;
        std::string gameType; // e.g., "sentence_match"

        std::string serialize() const override {
             std::vector<std::string> parts = {sessionToken, gameType};
             return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) gameType = parts[1];
        }
    };

    struct GameLevelDTO : public ISerializable {
        std::string id;
        std::string level; // "beginner", etc.
        std::string status; // "locked", "unlocked", "completed"

        std::string serialize() const override {
             std::vector<std::string> parts = {id, level, status};
             return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) id = parts[0];
            if (parts.size() >= 2) level = parts[1];
            if (parts.size() >= 3) status = parts[2];
        }
    };

    // GameDataRequest
    struct GameDataRequest : public ISerializable {
        std::string sessionToken;
        std::string gameId;

        std::string serialize() const override {
             std::vector<std::string> parts = {sessionToken, gameId};
             return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) gameId = parts[1];
        }
    };

    struct GameDataDTO : public ISerializable {
        std::string id;
        std::string type;
        std::string level;
        std::string questionJson; // The raw JSON content

        std::string serialize() const override {
             std::vector<std::string> parts = {id, type, level, questionJson};
             return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) id = parts[0];
            if (parts.size() >= 2) type = parts[1];
            if (parts.size() >= 3) level = parts[2];
            if (parts.size() >= 4) questionJson = parts[3];
        }
    };

    // GameSubmitRequest
    struct GameSubmitRequest : public ISerializable {
        std::string sessionToken;
        std::string gameId;
        std::string score;
        std::string detailsJson; // How they played, what they matched

         std::string serialize() const override {
             std::vector<std::string> parts = {sessionToken, gameId, score, detailsJson};
             return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) gameId = parts[1];
            if (parts.size() >= 3) score = parts[2];
            if (parts.size() >= 4) detailsJson = parts[3];
        }
    };

    // Game Management Payloads (Admin)
    struct GameCreateRequest : public ISerializable {
        std::string sessionToken;
        std::string type;
        std::string level;
        std::string questionJson;

        std::string serialize() const override {
             std::vector<std::string> parts = {sessionToken, type, level, questionJson};
             return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) type = parts[1];
            if (parts.size() >= 3) level = parts[2];
            if (parts.size() >= 4) questionJson = parts[3];
        }
    };

    struct GameDeleteRequest : public ISerializable {
        std::string sessionToken;
        std::string gameId;

        std::string serialize() const override {
             std::vector<std::string> parts = {sessionToken, gameId};
             return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) gameId = parts[1];
        }
    };

    struct GameUpdateRequest : public ISerializable {
        std::string sessionToken;
        std::string gameId;
        std::string type;
        std::string level;
        std::string questionJson;

        std::string serialize() const override {
             std::vector<std::string> parts = {sessionToken, gameId, type, level, questionJson};
             return utils::join(parts, '|');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, '|');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) gameId = parts[1];
            if (parts.size() >= 3) type = parts[2];
            if (parts.size() >= 4) level = parts[3];
            if (parts.size() >= 5) questionJson = parts[4];
        }
    };


    // Voice Call Payloads
    struct VoiceCallRequest : public ISerializable {
        std::string sessionToken;
        std::string targetUser;

        std::string serialize() const override {
            std::vector<std::string> parts = {sessionToken, targetUser};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) sessionToken = parts[0];
            if (parts.size() >= 2) targetUser = parts[1];
        }
    };

    struct VoiceCallNotification : public ISerializable {
        std::string callerUsername;
        std::string callerId;

        std::string serialize() const override {
            std::vector<std::string> parts = {callerUsername, callerId};
            return utils::join(parts, ';');
        }

        void deserialize(const std::string& raw) override {
            auto parts = utils::split(raw, ';');
            if (parts.size() >= 1) callerUsername = parts[0];
            if (parts.size() >= 2) callerId = parts[1];
        }
    };

} // namespace Payloads

#endif // COMMON_PAYLOADS_H
