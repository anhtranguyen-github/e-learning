#!/bin/bash

# Database setup script for English Learning Platform

echo "==================================="
echo "Database Setup for English Learning"
echo "==================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Database configuration
DB_NAME="english_learning"
DB_USER="postgres"
DB_PASSWORD="yourpass"

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

echo ""
echo -e "${YELLOW}Step 2: Creating database...${NC}"
sudo -u postgres psql -c "CREATE DATABASE $DB_NAME;" 2>/dev/null
if [ $? -eq 0 ]; then
    echo -e "${GREEN}Database '$DB_NAME' created successfully.${NC}"
else
    echo -e "${YELLOW}Database '$DB_NAME' already exists.${NC}"
fi

echo ""
echo -e "${YELLOW}Step 3: Initializing database schema...${NC}"
sudo -u postgres psql -d $DB_NAME -f init_db.sql
if [ $? -eq 0 ]; then
    echo -e "${GREEN}Database schema initialized successfully.${NC}"
else
    echo -e "${RED}Failed to initialize database schema.${NC}"
    exit 1
fi

echo ""
echo -e "${YELLOW}Step 4: Seeding database with test data...${NC}"
read -p "Do you want to seed the database with test data? (y/n): " seed_choice
if [ "$seed_choice" = "y" ] || [ "$seed_choice" = "Y" ]; then
    sudo -u postgres psql -d $DB_NAME -f seed_db.sql
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}Database seeded successfully.${NC}"
    else
        echo -e "${RED}Failed to seed database.${NC}"
    fi
else
    echo -e "${YELLOW}Skipping database seeding.${NC}"
fi

echo ""
echo -e "${YELLOW}Step 5: Testing database connection...${NC}"
if [ -f "db_test" ]; then
    ./db_test
else
    echo -e "${YELLOW}Test program not built. Building...${NC}"
    make
    if [ $? -eq 0 ]; then
        ./db_test
    else
        echo -e "${RED}Failed to build test program.${NC}"
    fi
fi

echo ""
echo -e "${GREEN}==================================="
echo "Database setup complete!"
echo "===================================${NC}"
echo ""
echo "Connection details:"
echo "  Database: $DB_NAME"
echo "  User: $DB_USER"
echo "  Host: localhost"
echo "  Port: 5432"
echo ""
echo "To view database contents:"
echo "  psql -U $DB_USER -d $DB_NAME"
