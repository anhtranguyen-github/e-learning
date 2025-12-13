#include "common/utils.h"
#include <random>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <fstream>

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

std::vector<char> readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        return {};
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
        return buffer;
    }
    return {};
}

std::string base64Encode(const std::vector<char>& data) {
    static const char* base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
        
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    for (char c : data) {
        char_array_3[i++] = c;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; (i < 4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';
            
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        
        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];
            
        while((i++ < 3))
            ret += '=';
    }
    
    return ret;
}

} // namespace utils
