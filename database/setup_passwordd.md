sudo -u postgres psql
ALTER USER postgres WITH PASSWORD 'yourpass';
\q



psql -U postgres -d english_learning -f database/init_db.sql
psql -U postgres -d english_learning -f database/seed_db.sql
psql -U postgres -d english_learning -f database/delete_db.sql
