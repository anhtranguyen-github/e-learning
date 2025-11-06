#include "database.h"

int main() {
    Database db("host=localhost port=5432 dbname=english_learning user=postgres password=yourpass");
    if (!db.connect()) return 1;

    db.execute("INSERT INTO users (username, password_hash, role) VALUES ('demo','123','student');");
    PGresult* res = db.query("SELECT * FROM users;");
    db.printResult(res);

    return 0;
}
