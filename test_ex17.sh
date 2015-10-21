#/usr/bin/env bash
set -e
DB_NAME='test_db'
EXEC='./ex17'
CLI="$EXEC $DB_NAME"

# Clean up from previous test
rm -f $DB_NAME

# Create db
$CLI c

# Set rows
$CLI s 1 roman roman
$CLI s 2 alex alex
$CLI s 3 boom boom

# Delete a row
$CLI d 3

# Set a row with a previously used id
$CLI s 3 fredek fredek

# Get rows
$CLI g 1
$CLI g 2
$CLI g 3

# List rows
$CLI l

