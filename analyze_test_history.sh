#!/bin/bash

# This script analyzes historical test logs to show trends and statistics

# Set terminal colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# Check if we have any archives
ARCHIVE_DIR="Logs/archive"
if [ ! -d "$ARCHIVE_DIR" ]; then
    echo -e "${RED}No archive directory found at ${ARCHIVE_DIR}${NC}"
    echo "Run tests with --archive option first:"
    echo "./run_all_tests_fixed --archive"
    exit 1
fi

# Count how many test runs we have archived
ARCHIVE_COUNT=$(find "$ARCHIVE_DIR" -maxdepth 1 -type d | wc -l)
ARCHIVE_COUNT=$((ARCHIVE_COUNT - 1)) # Subtract 1 for the archive dir itself
if [ "$ARCHIVE_COUNT" -eq 0 ]; then
    echo -e "${YELLOW}No test archives found.${NC}"
    echo "Run tests with --archive option first:"
    echo "./run_all_tests_fixed --archive"
    exit 1
fi

echo -e "${BOLD}=====================================================${NC}"
echo -e "${BOLD}KAI Test Suite History Analysis${NC}"
echo -e "${BOLD}=====================================================${NC}"
echo -e "Found ${ARCHIVE_COUNT} archived test runs"

# Extract test run dates
echo -e "\n${BOLD}Recent Test Runs:${NC}"
find "$ARCHIVE_DIR" -maxdepth 1 -type d -name "2*" | sort -r | head -10 | while read dir; do
    if [ -f "$dir/summary.log" ]; then
        RUN_DATE=$(grep "Generated:" "$dir/summary.log" | sed 's/Generated: //')
        TOTAL=$(grep "Total tests:" "$dir/summary.log" | awk '{print $3}')
        PASSED=$(grep "Passed tests:" "$dir/summary.log" | awk '{print $3}')
        PERCENT=$(grep "Passed tests:" "$dir/summary.log" | sed 's/.*(\(.*\)%)/\1/')
        STATUS=$(grep "Status:" "$dir/summary.log" | awk '{print $2}')
        
        DIR_NAME=$(basename "$dir")
        if [ "$STATUS" == "SUCCESS" ]; then
            echo -e "${GREEN}✓${NC} $DIR_NAME - $RUN_DATE - $PASSED/$TOTAL tests (${PERCENT}%)"
        else
            echo -e "${RED}✗${NC} $DIR_NAME - $RUN_DATE - $PASSED/$TOTAL tests (${PERCENT}%)"
        fi
    fi
done

# Calculate average pass rate
echo -e "\n${BOLD}Test Health Metrics:${NC}"
TOTAL_PERCENT=0
COUNT=0
for dir in $(find "$ARCHIVE_DIR" -maxdepth 1 -type d -name "2*" | sort); do
    if [ -f "$dir/summary.log" ]; then
        PERCENT=$(grep "Passed tests:" "$dir/summary.log" | sed 's/.*(\(.*\)%)/\1/')
        TOTAL_PERCENT=$(echo "$TOTAL_PERCENT + $PERCENT" | bc)
        COUNT=$((COUNT + 1))
    fi
done

if [ "$COUNT" -gt 0 ]; then
    AVG_PERCENT=$(echo "scale=2; $TOTAL_PERCENT / $COUNT" | bc)
    echo -e "Average pass rate: ${CYAN}${AVG_PERCENT}%${NC} (across $COUNT runs)"
    
    # Calculate trend
    FIRST_PERCENT=$(grep "Passed tests:" $(find "$ARCHIVE_DIR" -maxdepth 1 -type d -name "2*" | sort | head -1)/summary.log | sed 's/.*(\(.*\)%)/\1/')
    LAST_PERCENT=$(grep "Passed tests:" $(find "$ARCHIVE_DIR" -maxdepth 1 -type d -name "2*" | sort | tail -1)/summary.log | sed 's/.*(\(.*\)%)/\1/')
    CHANGE=$(echo "$LAST_PERCENT - $FIRST_PERCENT" | bc)
    
    if (( $(echo "$CHANGE > 0" | bc -l) )); then
        echo -e "Trend: ${GREEN}↑ Improving${NC} (+$CHANGE%)"
    elif (( $(echo "$CHANGE < 0" | bc -l) )); then
        echo -e "Trend: ${RED}↓ Declining${NC} ($CHANGE%)"
    else
        echo -e "Trend: → Stable (no change)"
    fi
fi

# Module health breakdown
echo -e "\n${BOLD}Module Health:${NC}"
for module in Core Rho Pi Tau Network; do
    PASS_COUNT=0
    TOTAL_COUNT=0
    
    for dir in $(find "$ARCHIVE_DIR" -maxdepth 1 -type d -name "2*" | sort); do
        if [ -f "$dir/summary.log" ]; then
            STATUS=$(grep "$module:" "$dir/summary.log" | grep -o "\[PASS\]\\|\[FAIL\]")
            TOTAL_COUNT=$((TOTAL_COUNT + 1))
            if [ "$STATUS" == "[PASS]" ]; then
                PASS_COUNT=$((PASS_COUNT + 1))
            fi
        fi
    done
    
    if [ "$TOTAL_COUNT" -gt 0 ]; then
        PERCENT=$(echo "scale=2; ($PASS_COUNT / $TOTAL_COUNT) * 100" | bc)
        echo -e "${module}: ${PASS_COUNT}/${TOTAL_COUNT} runs passed (${PERCENT}%)"
    fi
done

echo -e "\n${BOLD}=====================================================${NC}"
echo "For detailed test history, examine the logs in $ARCHIVE_DIR"
echo -e "${BOLD}=====================================================${NC}"

exit 0