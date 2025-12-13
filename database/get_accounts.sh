#!/bin/bash
DB_NAME="english_learning"

echo "========================================================"
echo " Demo Accounts"
echo "========================================================"
echo ""
sudo -u postgres psql -d $DB_NAME -c "SELECT username, role, password_hash as password FROM users WHERE username IN ('admin', 'teacher', 'student1', 'student2', 'student3') ORDER BY CASE role WHEN 'admin' THEN 1 WHEN 'teacher' THEN 2 ELSE 3 END, username;"
echo ""
echo "========================================================"
