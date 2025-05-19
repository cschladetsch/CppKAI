# KAI Test Suite Logging Format

The improved KAI test runner features enhanced logging capabilities with a structured format designed to make test output analysis easier.

## Log Organization

Each module has its own log file:

- `Logs/core_diagnostic.log` - Core tests
- `Logs/pi_diagnostic.log` - Pi language tests
- `Logs/rho_diagnostic.log` - Rho language tests 
- `Logs/tau_diagnostic.log` - Tau language tests
- `Logs/network_diagnostic.log` - Network tests
- `Logs/general_diagnostic.log` - General tests
- `Logs/summary.log` - Test suite summary

## Log File Format

Each log file follows a consistent format:

```
[Test Module Name] Diagnostic Log
Run Date: [Timestamp]
MODULE: [MODULE_NAME]
----------------------------------------
## TEST: [Test Binary Path] ##
## MODULE: [MODULE_NAME] ##
## Begin Output ##
[Test Output Content]
## End Output ##
----------------------------------------
```

## Summary Format

The summary log (`Logs/summary.log`) provides a comprehensive overview of the test run:

```
Test Suite Summary - [Timestamp]
====================================================
Status: [SUCCESS|FAILURE]

TEST COUNTS
----------------------------------------------------
Total tests:     [Total Test Count]
Passed tests:    [Passed Test Count]  ([Pass Percentage]%)
Failed tests:    [Failed Test Count]

MODULE DETAILS
----------------------------------------------------
Core:    [Core Test Count] tests [PASS|FAIL]
Rho:     [Rho Test Count] tests [PASS|FAIL]
Pi:      [Pi Test Count] tests [PASS|FAIL]
Tau:     [Tau Test Count] tests [PASS|FAIL]
Network: [Network Test Count] tests [PASS|FAIL]

LOG FILES
----------------------------------------------------
Core:    Logs/core_diagnostic.log
Rho:     Logs/rho_diagnostic.log
Pi:      Logs/pi_diagnostic.log
Tau:     Logs/tau_diagnostic.log
Network: Logs/network_diagnostic.log
====================================================
Generated: [Timestamp]
```

## Benefits of the New Logging Format

1. **Structured Organization** - Separating logs by module makes it easier to focus on specific parts of the system
2. **Timestamp Tracking** - Every log includes the date and time of execution for tracking test runs
3. **Clear Module Identification** - Module names are prominently displayed for quick identification
4. **Consistent Formatting** - All logs follow the same format for easier parsing and analysis
5. **Test Counts by Module** - The summary provides a breakdown of tests by module for better understanding of test coverage
6. **Success Percentage** - Pass rate percentage calculation helps track overall test health
7. **Pass/Fail Indicators** - Each module's status is clearly marked as PASS or FAIL
8. **Organized Sections** - Summary is divided into clear sections (Counts, Details, Log Files)
9. **Overall Status** - Top-level SUCCESS/FAILURE indicator provides immediate test health assessment
10. **Standardized Output Markers** - Begin/End markers in logs make parsing and automated analysis easier

## Using the Logs

To view the logs:

```bash
# View the summary
cat Logs/summary.log

# View specific module logs
cat Logs/rho_diagnostic.log
cat Logs/pi_diagnostic.log
# etc.
```

For filtering log content, standard Unix tools work well:

```bash
# Find test failures
grep -n "FAIL" Logs/rho_diagnostic.log

# View only test results 
grep -n "\[ RUN \]\\|\[  OK  \]\\|\[ FAIL \]" Logs/rho_diagnostic.log

# Extract just the content between BEGIN and END markers
sed -n '/## Begin Output ##/,/## End Output ##/p' Logs/rho_diagnostic.log

# Compare module passing rates
grep -h "tests \[PASS\]" Logs/summary.log | sort

# Find all passing modules
grep "PASS" Logs/summary.log
```

## Log Rotation and Archiving

The test runner automatically clears previous logs at the start of each test run. To preserve logs from a particular run, you can use the built-in archiving feature:

```bash
# Run tests and archive the results
./run_all_tests_fixed --archive
```

This will create a timestamped directory in `Logs/archive/` and copy all log files there. You can use these archived logs for historical analysis or troubleshooting.

If you want to manually preserve specific logs, copy them to a different location before running tests again:

```bash
# Manually create an archive directory with custom name
mkdir -p Logs/archive/important_milestone
cp Logs/*.log Logs/archive/important_milestone/
```

## Data Analysis and Visualization

The KAI test suite includes tools for analyzing historical test data. After you've archived some test runs, you can use the included analysis script:

```bash
# Analyze historical test data
./Scripts/analyze_test_history.sh
```

This script provides:
- A list of recent test runs with pass rates
- Overall test health metrics including average pass rates
- Trend analysis (improving, declining, or stable)
- Module-by-module health breakdown

For more advanced analysis, the structured format of the logs enables various approaches:

### Creating Historical Test Data

You can extract summary data for trend analysis:

```bash
# Extract test counts and dates
grep -h "Total tests:" Logs/archive/*/summary.log | awk '{print $3}' > total_tests_history.txt
grep -h "Timestamp" Logs/archive/*/summary.log | awk '{print $4 " " $5}' > test_dates.txt

# Extract pass percentages 
grep -h "Passed tests:" Logs/archive/*/summary.log | sed 's/.*(\(.*\)%)/\1/' > pass_percentage_history.txt
```

### Simple Visualization with Gnuplot

If you have Gnuplot installed, you can create simple visualizations:

```bash
# Create a script for plotting pass percentage over time
cat > plot_history.gnuplot << EOF
set terminal png
set output "test_history.png"
set title "KAI Test Suite Pass Rate Over Time"
set xlabel "Test Run"
set ylabel "Pass Percentage (%)"
set yrange [0:100]
plot "pass_percentage_history.txt" with linespoints title "Pass Rate"
EOF

# Run gnuplot
gnuplot plot_history.gnuplot
```

This produces a simple graph showing test health trends over time.