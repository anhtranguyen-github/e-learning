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
    // Session and login
    LOGIN_REQUEST = 100,
    LOGIN_SUCCESS = 101,
    LOGIN_FAILURE = 103,

    LOGOUT_REQUEST = 200,
    LOGOUT_SUCCESS = 201,

    // Lesson management
    LESSON_LIST_REQUEST = 110,
    LESSON_LIST_SUCCESS = 111,
    LESSON_LIST_FAILURE = 113,
    
    STUDY_LESSON_REQUEST = 120,
    STUDY_LESSON_SUCCESS = 121,
    STUDY_LESSON_FAILURE = 123,

    // Tests / Exercises
    MULTIPLE_CHOICE_REQUEST = 40,
    MULTIPLE_CHOICE_SUCCESS = 41,
    MULTIPLE_CHOICE_FAILURE = 43,
    FILL_IN_REQUEST = 50,
    FILL_IN_SUCCESS = 51,
    FILL_IN_FAILURE = 53,
    SENTENCE_ORDER_REQUEST = 60,
    SENTENCE_ORDER_SUCCESS = 61,
    SENTENCE_ORDER_FAILURE = 63,
    REWRITE_SENTENCE_REQUEST = 70,
    REWRITE_SENTENCE_SUCCESS = 71,
    REWRITE_SENTENCE_FAILURE = 73,
    WRITE_PARAGRAPH_REQUEST = 80,
    WRITE_PARAGRAPH_SUCCESS = 81,
    WRITE_PARAGRAPH_FAILURE = 83,
    SPEAKING_TOPIC_REQUEST = 90,
    SPEAKING_TOPIC_SUCCESS = 91,
    SPEAKING_TOPIC_FAILURE = 93,

    // Notifications
    NOTIFICATION_PUSH = 140,

    RESULT_LIST_REQUEST = 150,
    SUBMIT_ANSWER_REQUEST = 160,

    // Heartbeat
    HEARTBEAT = 900,

    // Graceful disconnect
    DISCONNECT_REQUEST = 901,
    DISCONNECT_ACK = 902,

    // General errors
    GENERAL_FAILURE = 9993,
    UNKNOWN_COMMAND_FAILURE = 9994
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
