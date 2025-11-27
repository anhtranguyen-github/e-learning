#include "../include/common/payloads.h"
#include "common/utils.h"
#include <iostream>
#include <cassert>

void test_utils() {
    std::string s = "a;b;c";
    auto parts = utils::split(s, ';');
    assert(parts.size() == 3);
    assert(parts[0] == "a");
    assert(parts[1] == "b");
    assert(parts[2] == "c");

    std::string joined = utils::join(parts, ';');
    assert(joined == s);
    
    std::cout << "Utils tests passed!" << std::endl;
}

void test_login_payload() {
    Payloads::LoginRequest req;
    req.username = "user1";
    req.password = "pass1";
    
    std::string serialized = req.serialize();
    assert(serialized == "user1;pass1");
    
    Payloads::LoginRequest req2;
    req2.deserialize(serialized);
    assert(req2.username == "user1");
    assert(req2.password == "pass1");
    
    std::cout << "LoginRequest tests passed!" << std::endl;
}

void test_empty_fields() {
    Payloads::LessonListRequest req;
    req.sessionToken = "token";
    req.topic = ""; // empty
    req.level = "1";
    
    std::string serialized = req.serialize();
    // "token;;1"
    
    Payloads::LessonListRequest req2;
    req2.deserialize(serialized);
    assert(req2.sessionToken == "token");
    assert(req2.topic == "");
    assert(req2.level == "1");
    
    std::cout << "Empty fields tests passed!" << std::endl;
}

void test_study_lesson_payload() {
    Payloads::StudyLessonRequest req;
    req.sessionToken = "token";
    req.lessonId = "123";
    req.lessonType = "video";
    
    std::string serialized = req.serialize();
    assert(serialized == "token;123;video");
    
    Payloads::StudyLessonRequest req2;
    req2.deserialize(serialized);
    assert(req2.sessionToken == "token");
    assert(req2.lessonId == "123");
    assert(req2.lessonType == "video");
    
    std::cout << "StudyLessonRequest tests passed!" << std::endl;
}

int main() {
    test_utils();
    test_login_payload();
    test_study_lesson_payload();
    test_empty_fields();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
