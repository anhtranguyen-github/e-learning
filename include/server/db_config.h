#ifndef DB_CONFIG_H
#define DB_CONFIG_H

#include <string>

namespace server {

class DBConfig {
public:
    static std::string getConnectionString() {
        // Default configuration
        // In production, read from environment variables or config file
        return "host=localhost port=5432 dbname=english_learning user=postgres password=yourpass";
    }
    
    static std::string getConnectionString(const std::string& host, 
                                          int port, 
                                          const std::string& dbname,
                                          const std::string& user,
                                          const std::string& password) {
        return "host=" + host + 
               " port=" + std::to_string(port) + 
               " dbname=" + dbname + 
               " user=" + user + 
               " password=" + password;
    }
};

} // namespace server

#endif // DB_CONFIG_H
