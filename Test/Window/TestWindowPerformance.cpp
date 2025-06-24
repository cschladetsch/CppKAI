#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <random>
#include <thread>

#include "NoConsoleWindow.h"

using namespace kai;
using namespace std;
using namespace std::chrono;

// Test fixture for Window Performance tests
class WindowPerformanceTest : public ::testing::Test {
   protected:
    std::unique_ptr<ExecutorWindow> window;
    std::mt19937 rng;

    void SetUp() override {
        window = std::make_unique<ExecutorWindow>();
        rng.seed(42);  // Deterministic seed for reproducible tests
    }

    void TearDown() override { window.reset(); }

    // Helper to measure execution time
    template <typename Func>
    double MeasureTime(Func func) {
        auto start = high_resolution_clock::now();
        func();
        auto end = high_resolution_clock::now();
        duration<double, std::milli> elapsed = end - start;
        return elapsed.count();
    }

    // Generate random command
    std::string GenerateRandomCommand() {
        std::uniform_int_distribution<> dist(1, 100);
        int a = dist(rng);
        int b = dist(rng);
        return std::to_string(a) + " " + std::to_string(b) + " +";
    }
};

// =============================================================================
// Command Execution Performance Tests
// =============================================================================

TEST_F(WindowPerformanceTest, CommandExecution_SingleCommand) {
    // Measure single command execution
    double time = MeasureTime([this]() { window->ExecCommand("1 2 +"); });

    // Should execute quickly (< 10ms for simple command)
    EXPECT_LT(time, 10.0);
}

TEST_F(WindowPerformanceTest, CommandExecution_BatchCommands) {
    const int NUM_COMMANDS = 1000;

    // Measure batch execution time
    double totalTime = MeasureTime([this, NUM_COMMANDS]() {
        for (int i = 0; i < NUM_COMMANDS; ++i) {
            window->ExecCommand(std::to_string(i).c_str());
        }
    });

    // Calculate average time per command
    double avgTime = totalTime / NUM_COMMANDS;

    // Average should be very low (< 1ms per command)
    EXPECT_LT(avgTime, 1.0);

    // Verify all commands executed
    EXPECT_EQ(window->History[Language::Pi].size(), NUM_COMMANDS);
}

TEST_F(WindowPerformanceTest, CommandExecution_ComplexExpressions) {
    std::vector<std::string> complexCommands = {
        "1 2 + 3 * 4 - 5 /", "10 { dup 1 - } 5 loop",
        "[ 1 2 3 4 5 ] { 2 * } map",
        "{ dup 0 > { dup 1 - fact * } { drop 1 } ifelse } 'fact = 10 fact"};

    double totalTime = 0;
    for (const auto& cmd : complexCommands) {
        totalTime +=
            MeasureTime([this, &cmd]() { window->ExecCommand(cmd.c_str()); });
    }

    // Complex commands should still be fast (< 50ms total)
    EXPECT_LT(totalTime, 50.0);
}

// =============================================================================
// Tab Switching Performance Tests
// =============================================================================

TEST_F(WindowPerformanceTest, TabSwitching_Speed) {
    const int NUM_SWITCHES = 1000;

    double time = MeasureTime([this, NUM_SWITCHES]() {
        for (int i = 0; i < NUM_SWITCHES; ++i) {
            window->SwitchTab(ConsoleTab::Pi);
            window->SwitchTab(ConsoleTab::Rho);
            window->SwitchTab(ConsoleTab::Debugger);
        }
    });

    // Tab switching should be instant (< 50ms for 1000 switches)
    EXPECT_LT(time, 50.0);
}

TEST_F(WindowPerformanceTest, TabSwitching_WithContent) {
    // Add content to all tabs
    window->ExecCommand("1 2 3 4 5");

    window->SwitchTab(ConsoleTab::Rho);
    window->ExecCommand("x = 10; y = 20; z = x + y");

    window->SwitchTab(ConsoleTab::Debugger);
    for (int i = 0; i < 100; ++i) {
        window->AddLog("Debug message %d", i);
    }

    // Measure switching with content
    const int NUM_SWITCHES = 100;
    double time = MeasureTime([this, NUM_SWITCHES]() {
        for (int i = 0; i < NUM_SWITCHES; ++i) {
            window->SwitchTab(static_cast<ConsoleTab>(i % 3));
        }
    });

    // Should still be fast even with content
    EXPECT_LT(time, 20.0);
}

// =============================================================================
// Memory Usage Tests
// =============================================================================

TEST_F(WindowPerformanceTest, Memory_HistoryGrowth) {
    const int NUM_COMMANDS = 10000;

    // Execute many commands to build history
    for (int i = 0; i < NUM_COMMANDS; ++i) {
        window->ExecCommand(GenerateRandomCommand().c_str());
    }

    // History should contain all commands
    EXPECT_EQ(window->History[Language::Pi].size(), NUM_COMMANDS);

    // Clear and verify memory is reclaimed
    window->History[Language::Pi].clear();
    EXPECT_TRUE(window->History[Language::Pi].empty());
}

TEST_F(WindowPerformanceTest, Memory_LogGrowth) {
    const int NUM_LOGS = 10000;

    // Add many log entries
    for (int i = 0; i < NUM_LOGS; ++i) {
        window->AddLog("Log entry %d with some additional text", i);
    }

    // Logs should contain all entries
    EXPECT_GE(window->Items[Language::Pi].size(), NUM_LOGS);

    // Clear logs
    window->ClearLog();
    EXPECT_TRUE(window->Items[Language::Pi].empty());
}

TEST_F(WindowPerformanceTest, Memory_StackGrowth) {
    const int NUM_ITEMS = 1000;

    // Push many items to stack
    double pushTime = MeasureTime([this, NUM_ITEMS]() {
        for (int i = 0; i < NUM_ITEMS; ++i) {
            window->ExecCommand(std::to_string(i).c_str());
        }
    });

    // Should handle large stack efficiently
    EXPECT_LT(pushTime, 100.0);
    // Note: Can't check actual stack size without Console

    // Clear by switching tabs (resets state)
    window->SwitchTab(ConsoleTab::Debugger);
    window->SwitchTab(ConsoleTab::Pi);
}

// =============================================================================
// Input Buffer Performance Tests
// =============================================================================

TEST_F(WindowPerformanceTest, InputBuffer_LargeInput) {
    // Test handling of maximum-size input
    std::string largeInput(sizeof(window->InputBuf) - 1, 'x');

    double time = MeasureTime([this, &largeInput]() {
        strcpy(window->InputBuf, largeInput.c_str());
        window->ExecCommand(window->InputBuf);
    });

    // Should handle large input without significant delay
    EXPECT_LT(time, 10.0);
}

TEST_F(WindowPerformanceTest, InputBuffer_MultilineLargeInput) {
    window->SwitchTab(ConsoleTab::Rho);

    // Create large multiline input
    std::stringstream ss;
    for (int i = 0; i < 100; ++i) {
        ss << "var" << i << " = " << i << ";\n";
    }
    std::string largeMultiline = ss.str();

    double time = MeasureTime([this, &largeMultiline]() {
        strcpy(window->MultilineInputBuf, largeMultiline.c_str());
        window->ExecCommand(window->MultilineInputBuf);
    });

    // Should handle large multiline input
    EXPECT_LT(time, 100.0);
}

// =============================================================================
// Concurrent Operations Tests
// =============================================================================

TEST_F(WindowPerformanceTest, Concurrent_RapidCommands) {
    // Simulate rapid command entry
    const int NUM_RAPID = 100;

    double time = MeasureTime([this, NUM_RAPID]() {
        for (int i = 0; i < NUM_RAPID; ++i) {
            window->ExecCommand(GenerateRandomCommand().c_str());
            // No delay between commands
        }
    });

    // Should handle rapid commands without queuing delays
    double avgTime = time / NUM_RAPID;
    EXPECT_LT(avgTime, 2.0);
}

TEST_F(WindowPerformanceTest, Concurrent_MixedOperations) {
    // Mix different operations rapidly
    double time = MeasureTime([this]() {
        for (int i = 0; i < 100; ++i) {
            // Execute command
            window->ExecCommand(std::to_string(i).c_str());

            // Switch tabs
            window->SwitchTab(static_cast<ConsoleTab>(i % 3));

            // Add log
            window->AddLog("Mixed operation %d", i);

            // Check history
            auto histSize = window->History[window->CurrentLanguage].size();
            (void)histSize;  // Avoid unused variable warning
        }
    });

    // Mixed operations should complete quickly
    EXPECT_LT(time, 200.0);
}

// =============================================================================
// Search and Filter Performance Tests
// =============================================================================

TEST_F(WindowPerformanceTest, Search_InLargeHistory) {
    // Build large history
    const int NUM_ENTRIES = 5000;
    for (int i = 0; i < NUM_ENTRIES; ++i) {
        window->ExecCommand(("test" + std::to_string(i)).c_str());
    }

    // Simulate searching through history
    double searchTime = MeasureTime([this]() {
        // Search for specific pattern
        int foundCount = 0;
        for (const auto& cmd : window->History[Language::Pi]) {
            if (cmd.find("test123") != std::string::npos) {
                foundCount++;
            }
        }
        EXPECT_GT(foundCount, 0);
    });

    // Search should be fast even in large history
    EXPECT_LT(searchTime, 10.0);
}

TEST_F(WindowPerformanceTest, Search_InLargeLogs) {
    // Build large log
    const int NUM_LOGS = 5000;
    for (int i = 0; i < NUM_LOGS; ++i) {
        window->AddLog("Log message number %d with searchable content", i);
    }

    // Search through logs
    double searchTime = MeasureTime([this, NUM_LOGS]() {
        int foundCount = 0;
        for (const auto& log : window->Items[Language::Pi]) {
            if (log.find("searchable") != std::string::npos) {
                foundCount++;
            }
        }
        EXPECT_EQ(foundCount, NUM_LOGS);
    });

    // Log search should be efficient
    EXPECT_LT(searchTime, 20.0);
}

// =============================================================================
// Stress Tests
// =============================================================================

TEST_F(WindowPerformanceTest, Stress_MaximumLoad) {
    // Execute everything at maximum speed
    auto stressTest = [this]() {
        // Commands
        for (int i = 0; i < 100; ++i) {
            window->ExecCommand(GenerateRandomCommand().c_str());
        }

        // Tab switches
        for (int i = 0; i < 50; ++i) {
            window->SwitchTab(static_cast<ConsoleTab>(i % 3));
        }

        // Logs
        for (int i = 0; i < 100; ++i) {
            window->AddLog("Stress test log %d", i);
        }

        // Clear operations
        window->ClearLog();
        window->ClearAllLogs();
    };

    double time = MeasureTime(stressTest);

    // Even under stress, should complete in reasonable time
    EXPECT_LT(time, 500.0);
}

TEST_F(WindowPerformanceTest, Stress_LongRunning) {
    // Simulate long-running session
    const int NUM_OPERATIONS = 10000;

    double totalTime = MeasureTime([this, NUM_OPERATIONS]() {
        for (int i = 0; i < NUM_OPERATIONS; ++i) {
            // Vary operations
            if (i % 10 == 0) {
                window->SwitchTab(static_cast<ConsoleTab>((i / 10) % 3));
            }

            if (i % 100 == 0) {
                window->ClearLog();
            }

            window->ExecCommand(std::to_string(i).c_str());
        }
    });

    // Average time per operation
    double avgTime = totalTime / NUM_OPERATIONS;

    // Should maintain performance over long session
    EXPECT_LT(avgTime, 1.0);
}

// =============================================================================
// Language-Specific Performance Tests
// =============================================================================

TEST_F(WindowPerformanceTest, Language_PiPerformance) {
    std::vector<std::string> piCommands = {
        "1 2 3 4 5 6 7 8 9 10 + + + + + + + + +", "100 { dup } 50 loop",
        "[ 1 2 3 4 5 ] { 2 * } map { 3 + } map",
        "\"Hello\" \" \" + \"World\" + \"!\" +", "true false and true or not"};

    double totalTime = 0;
    for (const auto& cmd : piCommands) {
        totalTime +=
            MeasureTime([this, &cmd]() { window->ExecCommand(cmd.c_str()); });
    }

    // Pi commands should execute efficiently
    EXPECT_LT(totalTime / piCommands.size(), 5.0);
}

TEST_F(WindowPerformanceTest, Language_RhoPerformance) {
    window->SwitchTab(ConsoleTab::Rho);

    std::vector<std::string> rhoCommands = {
        "x = 0; for (i = 1; i <= 100; i = i + 1) { x = x + i }",
        "fun fib(n) { if (n <= 1) { n } else { fib(n-1) + fib(n-2) } }; "
        "fib(10)",
        "arr = []; for (i = 0; i < 50; i = i + 1) { arr = arr + [i * 2] }",
        "s = \"\"; for (i = 0; i < 20; i = i + 1) { s = s + \"x\" }",
        "result = 1; i = 10; while (i > 0) { result = result * i; i = i - 1 }"};

    double totalTime = 0;
    for (const auto& cmd : rhoCommands) {
        totalTime +=
            MeasureTime([this, &cmd]() { window->ExecCommand(cmd.c_str()); });
    }

    // Rho commands should execute efficiently
    EXPECT_LT(totalTime / rhoCommands.size(), 50.0);
}

// =============================================================================
// Debugger Performance Tests
// =============================================================================

TEST_F(WindowPerformanceTest, Debugger_StepPerformance) {
    window->SwitchTab(ConsoleTab::Debugger);
    window->IsDebugging = true;

    // Add items to debug
    window->CurrentLanguage = Language::Pi;
    window->ExecCommand("1 2 3 4 5 6 7 8 9 10");

    // Measure stepping performance
    const int NUM_STEPS = 100;
    double time = MeasureTime([this, NUM_STEPS]() {
        for (int i = 0; i < NUM_STEPS; ++i) {
            window->ExecuteDebugStep();
        }
    });

    // Stepping should be fast
    EXPECT_LT(time / NUM_STEPS, 2.0);
}

TEST_F(WindowPerformanceTest, Debugger_LargeStackDisplay) {
    window->SwitchTab(ConsoleTab::Debugger);

    // Build large stack
    window->CurrentLanguage = Language::Pi;
    for (int i = 0; i < 1000; ++i) {
        window->ExecCommand(std::to_string(i).c_str());
    }

    // Measure time to display debug info
    double time = MeasureTime([this]() {
        // Simulate accessing debug information
        for (int i = 0; i < 100; ++i) {
            window->AddLog("Stack item %d: %d", i, i);
        }
    });

    // Stack access should be efficient
    EXPECT_LT(time, 50.0);
}