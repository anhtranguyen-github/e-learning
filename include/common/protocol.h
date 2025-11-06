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
    RESULT_LIST_FAILURE = 262,

    // Exam Management (270-289)
    EXAM_LIST_REQUEST = 270, // Re-adding this as it was removed earlier
    EXAM_LIST_SUCCESS = 271,
    EXAM_LIST_FAILURE = 272,

    // Notifications (290-299)
    NOTIFICATION_PUSH = 290,

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

    // Serialize: [2 bytes code][payload bytes...]
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> packet;
        packet.reserve(2 + data.size());
        uint16_t code_net = htons(static_cast<uint16_t>(code));
        uint8_t* p = reinterpret_cast<uint8_t*>(&code_net);
        packet.insert(packet.end(), p, p + 2);
        packet.insert(packet.end(), data.begin(), data.end());
        return packet;
    }

    // Deserialize from buffer
    static Message deserialize(const std::vector<uint8_t>& buffer) {
        if (buffer.size() < 2) {
            throw std::runtime_error("Invalid packet: too short");
        }
        uint16_t code_net;
        std::memcpy(&code_net, buffer.data(), 2);
        MsgCode c = static_cast<MsgCode>(ntohs(code_net));
        std::vector<uint8_t> payload(buffer.begin() + 2, buffer.end());
        return Message(c, payload);
    }
};

} // namespace protocol

#endif // PROTOCOL_H
