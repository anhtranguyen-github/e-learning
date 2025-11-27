#!/bin/bash

# ==========================================================
# Database setup script for English Learning Platform
# ==========================================================

echo "==================================="
echo "Database Setup for English Learning"
echo "==================================="

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Database configuration
DB_NAME="english_learning"
DB_USER="postgres"
DB_PASSWORD="yourpass"

# ==========================================================
# STEP 1: Check PostgreSQL installation
# ==========================================================
echo ""
echo -e "${YELLOW}Step 1: Checking PostgreSQL installation...${NC}"
if ! command -v psql &> /dev/null; then
    echo -e "${RED}PostgreSQL is not installed!${NC}"
    echo "Please install PostgreSQL:"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install postgresql postgresql-contrib libpq-dev"
    exit 1
fi
echo -e "${GREEN}PostgreSQL is installed.${NC}"

# ==========================================================
# STEP 2: Create database (if not exists)
# ==========================================================
echo ""
echo -e "${YELLOW}Step 2: Creating database '${DB_NAME}'...${NC}"
sudo -u postgres psql -tc "SELECT 1 FROM pg_database WHERE datname = '$DB_NAME'" | grep -q 1
if [ $? -ne 0 ]; then
    sudo -u postgres psql -c "CREATE DATABASE $DB_NAME;"
    echo -e "${GREEN}Database '$DB_NAME' created successfully.${NC}"
else
    echo -e "${YELLOW}Database '$DB_NAME' already exists.${NC}"
fi

# ==========================================================
# STEP 3: Optionally clean up existing tables
# ==========================================================
if [ -f "delete_db.sql" ]; then
    echo ""
    echo -e "${YELLOW}Step 3: Do you want to delete all existing tables?${NC}"
    read -p "(y/n): " delete_choice
    if [ "$delete_choice" = "y" ] || [ "$delete_choice" = "Y" ]; then
        echo -e "${YELLOW}Running delete_db.sql...${NC}"
        sudo -u postgres psql -d $DB_NAME -f delete_db.sql
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}Database cleared successfully.${NC}"
        else
            echo -e "${RED}Failed to execute delete_db.sql.${NC}"
            exit 1
        fi
    else
        echo -e "${YELLOW}Skipping database cleanup.${NC}"
    fi
else
    echo -e "${YELLOW}delete_db.sql not found, skipping cleanup.${NC}"
fi

# ==========================================================
# STEP 4: Initialize database schema
# ==========================================================
echo ""
echo -e "${YELLOW}Step 4: Initializing database schema...${NC}"
if [ -f "init_db.sql" ]; then
    sudo -u postgres psql -d $DB_NAME -f init_db.sql
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}Database schema initialized successfully.${NC}"
    else
        echo -e "${RED}Failed to initialize database schema.${NC}"
        exit 1
    fi
else
    echo -e "${RED}init_db.sql not found!${NC}"
    exit 1
fi

# ==========================================================
# STEP 5: Seed data (optional)
# ==========================================================
echo ""
echo -e "${YELLOW}Step 5: Seeding database with test data...${NC}"
SEEDED=false
if [ -f "seed_db.sql" ]; then
    # Auto-seed for now to ensure test data is present
    echo -e "${YELLOW}Running seed_db.sql...${NC}"
    sudo -u postgres psql -d $DB_NAME -f seed_db.sql
    if [ $? -eq 0 ]; then
        SEEDED=true
        echo -e "${GREEN}Database seeded successfully.${NC}"
    else
        echo -e "${RED}Failed to seed database.${NC}"
    fi
else
    echo -e "${YELLOW}seed_db.sql not found, skipping seeding.${NC}"
fi

# ==========================================================
# STEP 6: Query all tables (show summary)
# ==========================================================
if [ "$SEEDED" = true ]; then
    echo ""
    echo -e "${YELLOW}Step 6: Verifying seeded tables...${NC}"

    sudo -u postgres psql -d $DB_NAME -Atc "
        SELECT tablename FROM pg_tables
        WHERE schemaname = 'public'
        ORDER BY tablename;
    " | while read -r table; do
        echo -e "\n${GREEN}Table: $table${NC}"
        sudo -u postgres psql -d $DB_NAME -c "SELECT COUNT(*) AS row_count FROM $table;"
        sudo -u postgres psql -d $DB_NAME -c "SELECT * FROM $table LIMIT 5;"
    done
fi

# ==========================================================
# STEP 7: Test database connection
# ==========================================================
echo ""
echo -e "${YELLOW}Step 7: Testing database connection...${NC}"
if [ -f "db_test" ]; then
    ./db_test
else
    echo -e "${YELLOW}db_test not found. Attempting to build...${NC}"
    if command -v make &> /dev/null; then
        make db_test
        if [ -f "db_test" ]; then
            ./db_test
        else
            echo -e "${RED}Failed to build db_test.${NC}"
        fi
    else
        echo -e "${RED}Make not found. Cannot build db_test.${NC}"
    fi
fi

echo ""
echo -e "${GREEN}Database setup complete!${NC}"
echo "You can now run the server."
