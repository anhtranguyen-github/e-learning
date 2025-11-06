#include "server/database.h"
#include "common/logger.h"
#include <iostream>

namespace server {

Database::Database(const std::string& conninfo) : connInfo(conninfo), conn(nullptr) {
}

Database::~Database() {
    disconnect();
}

bool Database::connect() {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    if (conn && PQstatus(conn) == CONNECTION_OK) {
        return true;
    }
    
    conn = PQconnectdb(connInfo.c_str());
    
    if (PQstatus(conn) != CONNECTION_OK) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Connection to database failed: " + 
                                       std::string(PQerrorMessage(conn)));
        } else {
            std::cerr << "Connection to database failed: " << PQerrorMessage(conn);
        }
        return false;
    }
    
    if (logger::serverLogger) {
        logger::serverLogger->info("Connected to database successfully");
    } else {
        std::cout << "Connected to database successfully.\n";
    }
    return true;
}

void Database::disconnect() {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (conn) {
        PQfinish(conn);
        conn = nullptr;
    }
}

bool Database::isConnected() const {
    return conn != nullptr && PQstatus(conn) == CONNECTION_OK;
}

bool Database::execute(const std::string& sql) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    PGresult* res = PQexec(conn, sql.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        if (logger::serverLogger) {
            logger::serverLogger->error("SQL execution failed: " + 
                                       std::string(PQerrorMessage(conn)));
        } else {
            std::cerr << "SQL execution failed: " << PQerrorMessage(conn);
        }
        PQclear(res);
        return false;
    }
    PQclear(res);
    return true;
}

PGresult* Database::query(const std::string& sql) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    PGresult* res = PQexec(conn, sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Query failed: " + 
                                       std::string(PQerrorMessage(conn)));
        } else {
            std::cerr << "Query failed: " << PQerrorMessage(conn);
        }
        PQclear(res);
        return nullptr;
    }
    return res;
}

PGresult* Database::execParams(const std::string& sql, int nParams, const char* const* paramValues) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    PGresult* res = PQexecParams(conn, sql.c_str(), nParams, nullptr, paramValues, nullptr, nullptr, 0);
    
    ExecStatusType status = PQresultStatus(res);
    if (status != PGRES_TUPLES_OK && status != PGRES_COMMAND_OK) {
        if (logger::serverLogger) {
            logger::serverLogger->error("Parameterized query failed: " + 
                                       std::string(PQerrorMessage(conn)));
        } else {
            std::cerr << "Parameterized query failed: " << PQerrorMessage(conn);
        }
        PQclear(res);
        return nullptr;
    }
    return res;
}

void Database::printResult(PGresult* res) {
    if (!res) return;
    int nFields = PQnfields(res);
    int nRows = PQntuples(res);

    for (int i = 0; i < nFields; i++) {
        std::cout << PQfname(res, i) << "\t";
    }
    std::cout << "\n----------------------------------------\n";

    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nFields; j++) {
            std::cout << PQgetvalue(res, i, j) << "\t";
        }
        std::cout << "\n";
    }
    PQclear(res);
}

} // namespace server
