#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <sstream>

namespace utils {

// Generate a UUID-like session token
std::string generateSessionToken();

// Split string by delimiter
std::vector<std::string> split(const std::string& str, char delimiter);

// Extract cookie value from header
std::string extractCookie(const std::string& header, const std::string& cookieName);

// Create cookie header
std::string createCookieHeader(const std::string& sessionId);

// Parse login credentials from payload (format: "username;password")
bool parseLoginCredentials(const std::string& payload, std::string& username, std::string& password);

} // namespace utils

#endif // UTILS_H
