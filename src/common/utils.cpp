#include "common/utils.h"
#include <random>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace utils {

std::string generateSessionToken() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    const char* hex = "0123456789abcdef";
    
    // Generate UUID-like format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    for (int i = 0; i < 32; ++i) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            ss << '-';
        }
        ss << hex[dis(gen)];
    }
    
    return ss.str();
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string join(const std::vector<std::string>& v, char delimiter) {
    std::stringstream ss;
    for (size_t i = 0; i < v.size(); ++i) {
        ss << v[i];
        if (i != v.size() - 1) {
            ss << delimiter;
        }
    }
    return ss.str();
}

std::string extractCookie(const std::string& header, const std::string& cookieName) {
    // Look for "Cookie: session_id=<value>" or "session_id=<value>"
    size_t pos = header.find(cookieName + "=");
    if (pos == std::string::npos) {
        return "";
    }
    
    pos += cookieName.length() + 1; // Move past "session_id="
    size_t endPos = header.find_first_of("; \n\r", pos);
    
    if (endPos == std::string::npos) {
        return header.substr(pos);
    }
    
    return header.substr(pos, endPos - pos);
}

std::string createCookieHeader(const std::string& sessionId) {
    return "Set-Cookie: session_id=" + sessionId;
}

bool parseLoginCredentials(const std::string& payload, std::string& username, std::string& password) {
    size_t pos = payload.find(';');
    if (pos == std::string::npos) {
        return false;
    }
    
    username = payload.substr(0, pos);
    password = payload.substr(pos + 1);
    
    // Trim whitespace
    username.erase(0, username.find_first_not_of(" \t\n\r"));
    username.erase(username.find_last_not_of(" \t\n\r") + 1);
    password.erase(0, password.find_first_not_of(" \t\n\r"));
    password.erase(password.find_last_not_of(" \t\n\r") + 1);
    
    return !username.empty() && !password.empty();
}

} // namespace utils
