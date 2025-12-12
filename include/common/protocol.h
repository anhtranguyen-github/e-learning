#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <arpa/inet.h>

namespace protocol {

// Message codes for client-server communication
enum class MsgCode : uint16_t {
    // Session and Login (100-109)
    LOGIN_REQUEST = 100,
    LOGIN_SUCCESS = 101,
    LOGIN_FAILURE = 102,
    REGISTER_REQUEST = 103,
    REGISTER_SUCCESS = 104,
    REGISTER_FAILURE = 105,

    // Logout (110-119)
    LOGOUT_REQUEST = 110,
    LOGOUT_SUCCESS = 111,

    // Lesson Management (120-149)
    LESSON_LIST_REQUEST = 120,
    LESSON_LIST_SUCCESS = 121,
    LESSON_LIST_FAILURE = 122,
    
    STUDY_LESSON_REQUEST = 130,
    STUDY_LESSON_SUCCESS = 131,
    STUDY_LESSON_FAILURE = 132,

    // General Exercise Management (150-169)
    EXERCISE_LIST_REQUEST = 150,
    EXERCISE_LIST_SUCCESS = 151,
    EXERCISE_LIST_FAILURE = 152,

    STUDY_EXERCISE_REQUEST = 160,
    STUDY_EXERCISE_SUCCESS = 161,
    STUDY_EXERCISE_FAILURE = 162,

    // Specific Exercise Types (170-249)
    MULTIPLE_CHOICE_REQUEST = 170,
    MULTIPLE_CHOICE_SUCCESS = 171,
    MULTIPLE_CHOICE_FAILURE = 172,

    FILL_IN_REQUEST = 180,
    FILL_IN_SUCCESS = 181,
    FILL_IN_FAILURE = 182,

    SENTENCE_ORDER_REQUEST = 190,
    SENTENCE_ORDER_SUCCESS = 191,
    SENTENCE_ORDER_FAILURE = 192,

    REWRITE_SENTENCE_REQUEST = 200,
    REWRITE_SENTENCE_SUCCESS = 201,
    REWRITE_SENTENCE_FAILURE = 202,

    WRITE_PARAGRAPH_REQUEST = 210,
    WRITE_PARAGRAPH_SUCCESS = 211,
    WRITE_PARAGRAPH_FAILURE = 212,

    SPEAKING_TOPIC_REQUEST = 220,
    SPEAKING_TOPIC_SUCCESS = 221,
    SPEAKING_TOPIC_FAILURE = 222,

    // Submission and Results (250-269)
    SUBMIT_ANSWER_REQUEST = 250,
    SUBMIT_ANSWER_SUCCESS = 251,
    SUBMIT_ANSWER_FAILURE = 252,

    RESULT_LIST_REQUEST = 260,
    RESULT_LIST_SUCCESS = 261,
    RESULT_DETAIL_REQUEST = 262,
    RESULT_DETAIL_SUCCESS = 263,
    RESULT_DETAIL_FAILURE = 269, // New code
    RESULT_LIST_FAILURE = 264,
    RESULT_REQUEST = 265, // Added for single result request
    
    // Teacher Grading (266-269)
    PENDING_SUBMISSIONS_REQUEST = 266,
    PENDING_SUBMISSIONS_SUCCESS = 267,
    PENDING_SUBMISSIONS_FAILURE = 268,
    
    GRADE_SUBMISSION_REQUEST = 255,
    GRADE_SUBMISSION_SUCCESS = 253, // Reusing submission success range or new one
    GRADE_SUBMISSION_FAILURE = 254,

    // Exam Management (270-289)
    EXAM_LIST_REQUEST = 270, // Re-adding this as it was removed earlier
    EXAM_LIST_SUCCESS = 271,
    EXAM_LIST_FAILURE = 272,
    EXAM_REQUEST = 275, // Added for single exam request
    EXAM_SUCCESS = 276,
    EXAM_FAILURE = 277,
    EXAM_ALREADY_TAKEN = 278,

    // Notifications (290-299)
    NOTIFICATION_PUSH = 290,

    // Chat Management (300-319)
    SEND_CHAT_PRIVATE_REQUEST = 300,
    CHAT_PRIVATE_RECEIVE = 301,
    CHAT_MESSAGE_SUCCESS = 302,
    CHAT_MESSAGE_FAILURE = 303,
    CHAT_HISTORY_REQUEST = 304,
    CHAT_HISTORY_SUCCESS = 305,
    CHAT_HISTORY_FAILURE = 306,
    RECENT_CHATS_REQUEST = 307,
    RECENT_CHATS_SUCCESS = 308,
    RECENT_CHATS_FAILURE = 309,

    // Voice Call Management (310-329)
    CALL_INITIATE_REQUEST = 310,
    CALL_INCOMING = 311,
    CALL_ANSWER_REQUEST = 312,
    CALL_DECLINE_REQUEST = 313,
    CALL_END_REQUEST = 314,
    CALL_ENDED = 315,
    CALL_BUSY = 316,
    CALL_FAILED = 317,

    // Heartbeat and Disconnect (900-909)
    HEARTBEAT = 900,
    DISCONNECT_REQUEST = 901,
    DISCONNECT_ACK = 902,

    // General Errors (990-999)
    GENERAL_FAILURE = 990,
    UNKNOWN_COMMAND_FAILURE = 991
};

// Message structure for network communication
struct Message {
    MsgCode code;
    std::vector<uint8_t> data;

    Message() = default;
    Message(MsgCode c, const std::vector<uint8_t>& d) : code(c), data(d) {}
    Message(MsgCode c, const std::string& s)
        : code(c), data(s.begin(), s.end()) {}

    // Convert data to string
    std::string toString() const {
        return std::string(data.begin(), data.end());
    }

    // Serialize: [4 bytes length][2 bytes code][payload bytes...]
    // Length includes the 4 bytes of length field itself.
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> packet;
        uint32_t total_len = 4 + 2 + data.size();
        packet.reserve(total_len);
        
        uint32_t len_net = htonl(total_len);
        uint8_t* p_len = reinterpret_cast<uint8_t*>(&len_net);
        packet.insert(packet.end(), p_len, p_len + 4);

        uint16_t code_net = htons(static_cast<uint16_t>(code));
        uint8_t* p_code = reinterpret_cast<uint8_t*>(&code_net);
        packet.insert(packet.end(), p_code, p_code + 2);
        
        packet.insert(packet.end(), data.begin(), data.end());
        return packet;
    }

    // Deserialize from buffer
    // Expects buffer to start with a complete message.
    static Message deserialize(const std::vector<uint8_t>& buffer) {
        if (buffer.size() < 6) { // 4 bytes length + 2 bytes code
            throw std::runtime_error("Invalid packet: too short");
        }
        
        uint32_t len_net;
        std::memcpy(&len_net, buffer.data(), 4);
        uint32_t total_len = ntohl(len_net);
        
        if (buffer.size() < total_len) {
            throw std::runtime_error("Invalid packet: incomplete");
        }

        uint16_t code_net;
        std::memcpy(&code_net, buffer.data() + 4, 2);
        MsgCode c = static_cast<MsgCode>(ntohs(code_net));
        
        // Payload starts at offset 6, length is total_len - 6
        std::vector<uint8_t> payload(buffer.begin() + 6, buffer.begin() + total_len);
        return Message(c, payload);
    }

    // Helper to check if buffer has a full message
    // Returns 0 if not enough data to determine length, or if incomplete.
    // Returns total message length if complete message is present.
    static uint32_t getFullLength(const std::vector<uint8_t>& buffer) {
        if (buffer.size() < 4) return 0;
        
        uint32_t len_net;
        std::memcpy(&len_net, buffer.data(), 4);
        uint32_t total_len = ntohl(len_net);
        
        if (buffer.size() >= total_len) {
            return total_len;
        }
        return 0;
    }
};

} // namespace protocol

#endif // PROTOCOL_H
