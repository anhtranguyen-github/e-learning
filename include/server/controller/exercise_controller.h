#ifndef EXERCISE_CONTROLLER_H
#define EXERCISE_CONTROLLER_H

#include "common/protocol.h"
#include "server/session.h"
#include "server/repository/exercise_repository.h"
#include <memory>
#include <string>

namespace server {

/**
 * ExerciseHandler - Handles exercise-related messages from clients
 *
 * This class processes EXERCISE_LIST_REQUEST and STUDY_EXERCISE_REQUEST messages,
 * validates session tokens, loads exercises from database, and sends responses.
 */
class ExerciseController {
private:
    std::shared_ptr<SessionManager> sessionManager;
    std::shared_ptr<ExerciseRepository> exerciseRepository;

    bool sendMessage(int clientFd, const protocol::Message& msg);

    // Helper to parse exercise type string
    ExerciseType parseExerciseType(const std::string& typeStr);

public:
    /**
     * Constructor
     * @param sm - Shared pointer to SessionManager for token validation
     * @param er - Shared pointer to ExerciseRepository for database operations
     */
    ExerciseController(std::shared_ptr<SessionManager> sm, std::shared_ptr<ExerciseRepository> er);

    /**
     * Handle EXERCISE_LIST_REQUEST message
     *
     * Expected payload format: <session_token>[;<type>;<level>;<lesson_id>]
     * - session_token: User's session token for authentication
     * - type: Optional filter by exercise type (empty = no filter)
     * - level: Optional filter by level (empty = no filter)
     * - lesson_id: Optional filter by lesson ID (-1 = no filter)
     *
     * Flow:
     * 1. Parse session token from payload
     * 2. Validate token/session
     * 3. Parse optional filters (type, level, lesson_id)
     * 4. Load exercises from database via ExerciseRepository
     * 5. Apply filters if provided
     * 6. Serialize exercise list
     * 7. Send EXERCISE_LIST_SUCCESS with serialized data
     * 8. On error, send EXERCISE_LIST_FAILURE
     *
     * @param clientFd - Client socket file descriptor
     * @param msg - Incoming message from client
     */
    void handleExerciseListRequest(int clientFd, const protocol::Message& msg);

    /**
     * Handle STUDY_EXERCISE_REQUEST message
     *
     * Expected payload format: <session_token>;<exercise_id>;<exercise_type>
     * - session_token: User's session token for authentication
     * - exercise_id: ID of the exercise to study
     * - exercise_type: Type of content to retrieve (question, options, answer, explanation, full)
     *
     * Flow:
     * 1. Parse payload (token, exercise_id, exercise_type)
     * 2. Validate token/session
     * 3. Load specific exercise from database by ID
     * 4. Serialize only the requested content type
     * 5. Send STUDY_EXERCISE_SUCCESS with serialized content
     * 6. On error, send STUDY_EXERCISE_FAILURE
     *
     * @param clientFd - Client socket file descriptor
     * @param msg - Incoming message from client
     */
    void handleStudyExerciseRequest(int clientFd, const protocol::Message& msg);

    /**
     * Handle specific exercise requests (MULTIPLE_CHOICE, FILL_IN, etc.)
     *
     * Expected payload format: <session_token>;<exercise_id>
     * The response code will match the request code + 1 (e.g. REQUEST -> SUCCESS).
     */
    void handleSpecificExerciseRequest(int clientFd, const protocol::Message& msg);
};

} // namespace server

#endif // EXERCISE_CONTROLLER_H