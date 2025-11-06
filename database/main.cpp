#include "database.h"
#include <iostream>

int main() {
    std::cout << "=== PostgreSQL Database Test ===" << std::endl;
    
    Database db("host=localhost port=5432 dbname=english_learning user=postgres password=yourpass");
    
    if (!db.connect()) {
        std::cerr << "Failed to connect to database" << std::endl;
        return 1;
    }
    
    std::cout << "\n--- Testing User Insertion ---" << std::endl;
    db.execute("INSERT INTO users (username, password_hash, role, level) VALUES ('demo','demo123','student','beginner') ON CONFLICT (username) DO NOTHING;");
    db.execute("INSERT INTO users (username, password_hash, role, level) VALUES ('test','test123','student','intermediate') ON CONFLICT (username) DO NOTHING;");
    db.execute("INSERT INTO users (username, password_hash, role, level) VALUES ('admin','admin123','admin','advanced') ON CONFLICT (username) DO NOTHING;");
    
    std::cout << "\n--- Querying All Users ---" << std::endl;
    PGresult* res = db.query("SELECT user_id, username, role, level, created_at FROM users;");
    if (res) {
        db.printResult(res);
    }
    
    std::cout << "\n--- Testing Session Creation ---" << std::endl;
    db.execute("INSERT INTO server_sessions (session_id, user_id, client_fd, active) VALUES ('test_session_123', 1, 5, TRUE) ON CONFLICT (session_id) DO NOTHING;");
    
    std::cout << "\n--- Querying Active Sessions ---" << std::endl;
    PGresult* sessRes = db.query("SELECT session_id, user_id, client_fd, last_active, active FROM server_sessions WHERE active = TRUE;");
    if (sessRes) {
        db.printResult(sessRes);
    }
    
    std::cout << "\n=== Database Test Complete ===" << std::endl;
    
    return 0;
}
