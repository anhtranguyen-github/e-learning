#ifndef LESSON_CONTROLLER_H
#define LESSON_CONTROLLER_H

#include "common/protocol.h"
#include "server/session.h"
#include "server/repository/lesson_repository.h"
#include <memory>
#include <string>

namespace server {

/**
 * LessonController - Handles lesson-related messages from clients
 *
 * This class processes LESSON_LIST_REQUEST and STUDY_LESSON_REQUEST messages,
 * validates session tokens, loads lessons from database, and sends responses.
 */
class LessonController {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<LessonRepository> lessonRepository;

    // Helper function to send a message to a client
    bool sendMessage(int clientFd, const protocol::Message& msg);
    
    // Helper to parse lesson type string
    LessonType parseLessonType(const std::string& typeStr);

public:
    /**
     * Constructor
     * @param sm - Shared pointer to SessionManager for token validation
     * @param lr - Shared pointer to LessonRepository for database operations
     */
    LessonController(std::shared_ptr<SessionManager> sm, std::shared_ptr<LessonRepository> lr);

    /**
     * Handle LESSON_LIST_REQUEST message
     * 
     * Expected payload format: <session_token>[;<topic>;<level>]
     * - session_token: User's session token for authentication
     * - topic: Optional filter by topic (empty = no filter)
     * - level: Optional filter by level (empty = no filter)
     * 
     * Flow:
     * 1. Parse session token from payload
     * 2. Validate token/session
     * 3. Parse optional filters (topic, level)
     * 4. Load lessons from database via LessonRepository
     * 5. Apply filters if provided
     * 6. Serialize lesson list
     * 7. Send LESSON_LIST_SUCCESS with serialized data
     * 8. On error, send LESSON_LIST_FAILURE
     * 
     * @param clientFd - Client socket file descriptor
     * @param msg - Incoming message from client
     */
    void handleLessonListRequest(int clientFd, const protocol::Message& msg);

    /**
     * Handle STUDY_LESSON_REQUEST message
     * 
     * Expected payload format: <session_token>;<lesson_id>;<lesson_type>
     * - session_token: User's session token for authentication
     * - lesson_id: ID of the lesson to study
     * - lesson_type: Type of content to retrieve (video, audio, text, vocabulary, grammar, full)
     * 
     * Flow:
     * 1. Parse payload (token, lesson_id, lesson_type)
     * 2. Validate token/session
     * 3. Load specific lesson from database by ID
     * 4. Serialize only the requested content type
     * 5. Send STUDY_LESSON_SUCCESS with serialized content
     * 6. On error, send STUDY_LESSON_FAILURE
     * 
     * @param clientFd - Client socket file descriptor
     * @param msg - Incoming message from client
     */
    void handleStudyLessonRequest(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // LESSON_CONTROLLER_H
