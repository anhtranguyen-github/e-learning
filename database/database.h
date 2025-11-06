#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <libpq-fe.h>

class Database {
private:
    PGconn* conn;

public:
    Database(const std::string& conninfo);
    ~Database();

    bool connect();
    void disconnect();

    bool execute(const std::string& sql);
    PGresult* query(const std::string& sql);

    void printResult(PGresult* res);
};

#endif // DATABASE_H
