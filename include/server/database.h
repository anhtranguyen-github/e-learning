#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <postgresql/libpq-fe.h>
#include <mutex>

namespace server {

class Database {
private:
    PGconn* conn;
    std::string connInfo;
    std::mutex dbMutex;

public:
    Database(const std::string& conninfo);
    ~Database();

    bool connect();
    void disconnect();
    bool isConnected() const;

    bool execute(const std::string& sql);
    PGresult* query(const std::string& sql);
    
    // Prepared statement support
    PGresult* execParams(const std::string& sql, int nParams, const char* const* paramValues);

    void printResult(PGresult* res);
    
    std::mutex& getMutex() { return dbMutex; }
    PGconn* getConnection() const { return conn; }
};

} // namespace server

#endif // DATABASE_H
