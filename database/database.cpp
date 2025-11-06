#include "database.h"
#include <iostream>

Database::Database(const std::string& conninfo) : conn(nullptr) {
    conn = PQconnectdb(conninfo.c_str());
}

Database::~Database() {
    disconnect();
}

bool Database::connect() {
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Connection to database failed: "
                  << PQerrorMessage(conn);
        return false;
    }
    std::cout << "Connected to database successfully.\n";
    return true;
}

void Database::disconnect() {
    if (conn) {
        PQfinish(conn);
        conn = nullptr;
    }
}

bool Database::execute(const std::string& sql) {
    PGresult* res = PQexec(conn, sql.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "SQL execution failed: " << PQerrorMessage(conn);
        PQclear(res);
        return false;
    }
    PQclear(res);
    return true;
}

PGresult* Database::query(const std::string& sql) {
    PGresult* res = PQexec(conn, sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Query failed: " << PQerrorMessage(conn);
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
