#!/bin/bash

# Configuration
DB_NAME="english_learning"
GREEN='\033[0;32m'
NC='\033[0m' # No Color

function show_usage {
    echo "Usage: ./setup.sh [init|reset]"
    echo "  init  : Create database (if missing), load schema and seed data."
    echo "  reset : Drop all existing data (tables), reload schema and seed data."
}

if [ "$1" == "reset" ]; then
    echo -e "${GREEN}Resetting database '$DB_NAME'...${NC}"
    # Drop tables
    if [ -f "delete_db.sql" ]; then
        sudo -u postgres psql -d $DB_NAME -f delete_db.sql
    fi
    # Init and Seed
    sudo -u postgres psql -d $DB_NAME -f init_db.sql
    sudo -u postgres psql -d $DB_NAME -f seed.sql
    echo -e "${GREEN}Database reset complete!${NC}"

elif [ "$1" == "init" ]; then
    echo -e "${GREEN}Initializing database '$DB_NAME'...${NC}"
    # Create DB if not exists
    sudo -u postgres psql -tc "SELECT 1 FROM pg_database WHERE datname = '$DB_NAME'" | grep -q 1 || sudo -u postgres psql -c "CREATE DATABASE $DB_NAME;"
    
    # Init Schema
    sudo -u postgres psql -d $DB_NAME -f init_db.sql
    # Seed Data
    sudo -u postgres psql -d $DB_NAME -f seed.sql
    echo -e "${GREEN}Database initialization complete!${NC}"

else
    show_usage
    exit 1
fi
