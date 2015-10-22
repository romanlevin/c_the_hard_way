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

# Can't set an existing row
set +e
$CLI s 3 blah blah >/dev/null
case $? in
        1) : ;;
        *) echo "ohnoes $?"; exit 1;;
esac
set -e

# Get rows
$CLI g 1 >/dev/null
$CLI g 2 >/dev/null
$CLI g 3 >/dev/null

# List rows
$CLI l >/dev/null

echo PASS

