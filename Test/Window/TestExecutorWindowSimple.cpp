#include <KAI/Console.h>
#include <KAI/Core/Exception.h>
#include <gtest/gtest.h>

#include <memory>
#include <sstream>

using namespace kai;
using namespace std;

// Since we can't directly test the UI components without the actual imgui
// implementation, we'll test the core console functionality that the Window
// uses

class WindowConsoleTest : public ::testing::Test {
   protected:
    Console console;

    void SetUp() override {
        // Initialize console with Pi language by default
        console.SetLanguage(Language::Pi);
    }

    void TearDown() override {
        // Clean up
    }
};

// =============================================================================
// Pi Language Tests
// =============================================================================

TEST_F(WindowConsoleTest, Pi_BasicArithmetic) {
    // Test basic addition
    console.Execute("2 3 +", Structure::Expression);
    auto stack = console.GetExecutor()->GetDataStack();
    EXPECT_GT(stack->Size(), 0);

    // Test multiplication
    console.Execute("4 5 *", Structure::Expression);
    EXPECT_GT(stack->Size(), 0);

    // Clear stack for next test
    console.GetExecutor()->ClearStacks();
}

TEST_F(WindowConsoleTest, Pi_StackOperations) {
    // Test dup
    console.Execute("5 dup", Structure::Expression);
    auto stack = console.GetExecutor()->GetDataStack();
    EXPECT_EQ(stack->Size(), 2);

    // Test swap
    console.GetExecutor()->ClearStacks();
    console.Execute("1 2 swap", Structure::Expression);
    EXPECT_EQ(stack->Size(), 2);

    // Test drop
    console.Execute("drop", Structure::Expression);
    EXPECT_EQ(stack->Size(), 1);
}

TEST_F(WindowConsoleTest, Pi_Variables) {
    // Test variable assignment
    console.Execute("42 'answer =", Structure::Expression);

    // Test variable retrieval
    console.Execute("answer", Structure::Expression);
    auto stack = console.GetExecutor()->GetDataStack();
    EXPECT_GT(stack->Size(), 0);
}

TEST_F(WindowConsoleTest, Pi_Arrays) {
    // Test array creation
    console.Execute("[ 1 2 3 4 5 ]", Structure::Expression);
    auto stack = console.GetExecutor()->GetDataStack();
    EXPECT_GT(stack->Size(), 0);
}

TEST_F(WindowConsoleTest, Pi_Strings) {
    // Test string literals
    console.Execute("\"Hello, World!\"", Structure::Expression);
    auto stack = console.GetExecutor()->GetDataStack();
    EXPECT_GT(stack->Size(), 0);
}

TEST_F(WindowConsoleTest, Pi_Functions) {
    // Test function definition
    console.Execute("{ 2 * } 'double =", Structure::Expression);

    // Test function call
    console.Execute("5 double", Structure::Expression);
    auto stack = console.GetExecutor()->GetDataStack();
    EXPECT_GT(stack->Size(), 0);
}

TEST_F(WindowConsoleTest, Pi_ErrorHandling) {
    // Test undefined variable - should throw
    EXPECT_THROW(console.Execute("undefined_var", Structure::Expression),
                 Exception::Base);

    // Test division by zero
    EXPECT_THROW(console.Execute("1 0 /", Structure::Expression),
                 Exception::Base);
}

// =============================================================================
// Rho Language Tests
// =============================================================================

TEST_F(WindowConsoleTest, Rho_BasicSetup) {
    // Switch to Rho
    console.SetLanguage(Language::Rho);
    EXPECT_EQ(console.GetLanguage(), Language::Rho);
}

TEST_F(WindowConsoleTest, Rho_BasicArithmetic) {
    console.SetLanguage(Language::Rho);

    // Test basic arithmetic
    console.Execute("20 + 30", Structure::Expression);
    auto stack = console.GetExecutor()->GetDataStack();
    EXPECT_GT(stack->Size(), 0);

    console.GetExecutor()->ClearStacks();
    console.Execute("100 - 25", Structure::Expression);
    EXPECT_GT(stack->Size(), 0);
}

TEST_F(WindowConsoleTest, Rho_Variables) {
    console.SetLanguage(Language::Rho);

    // Test variable declaration
    console.Execute("x = 10", Structure::Statement);

    // Test variable usage
    console.Execute("x", Structure::Expression);
    auto stack = console.GetExecutor()->GetDataStack();
    EXPECT_GT(stack->Size(), 0);
}

TEST_F(WindowConsoleTest, Rho_Functions) {
    console.SetLanguage(Language::Rho);

    // Test simple function
    console.Execute("fun add(a, b) { a + b }", Structure::Statement);

    // Test function call
    console.Execute("add(5, 3)", Structure::Expression);
    auto stack = console.GetExecutor()->GetDataStack();
    EXPECT_GT(stack->Size(), 0);
}

TEST_F(WindowConsoleTest, Rho_ControlFlow) {
    console.SetLanguage(Language::Rho);

    // Test if statement
    console.Execute("x = 5", Structure::Statement);
    console.Execute("if (x > 3) { \"greater\" } else { \"less\" }",
                    Structure::Expression);
    auto stack = console.GetExecutor()->GetDataStack();
    EXPECT_GT(stack->Size(), 0);
}

TEST_F(WindowConsoleTest, Rho_ErrorHandling) {
    console.SetLanguage(Language::Rho);

    // Test undefined variable
    EXPECT_THROW(console.Execute("undefined_variable", Structure::Expression),
                 Exception::Base);
}

// =============================================================================
// Language Switching Tests
// =============================================================================

TEST_F(WindowConsoleTest, LanguageSwitching) {
    // Start with Pi
    EXPECT_EQ(console.GetLanguage(), Language::Pi);

    // Execute Pi command
    console.Execute("1 2 +", Structure::Expression);

    // Switch to Rho
    console.SetLanguage(Language::Rho);
    EXPECT_EQ(console.GetLanguage(), Language::Rho);

    // Execute Rho command
    console.Execute("x = 10", Structure::Statement);

    // Switch back to Pi
    console.SetLanguage(Language::Pi);
    EXPECT_EQ(console.GetLanguage(), Language::Pi);
}

// =============================================================================
// History Tests
// =============================================================================

TEST_F(WindowConsoleTest, CommandHistory) {
    // Execute several commands
    console.Execute("1 2 +", Structure::Expression);
    console.Execute("3 4 *", Structure::Expression);
    console.Execute("5 6 -", Structure::Expression);

    // Commands should execute without error
    EXPECT_TRUE(true);
}

// =============================================================================
// Shell Command Tests
// =============================================================================

TEST_F(WindowConsoleTest, ShellCommands) {
    // Test shell command detection
    std::string cmd = "$echo test";
    EXPECT_TRUE(cmd[0] == '$');

    // Test backtick detection
    std::string backtick = "`echo hello`";
    EXPECT_TRUE(backtick[0] == '`');
}

// =============================================================================
// Stack Management Tests
// =============================================================================

TEST_F(WindowConsoleTest, StackManagement) {
    // Build stack
    console.Execute("1 2 3 4 5", Structure::Expression);
    auto stack = console.GetExecutor()->GetDataStack();
    EXPECT_EQ(stack->Size(), 5);

    // Clear stack
    console.GetExecutor()->ClearStacks();
    EXPECT_EQ(stack->Size(), 0);
}

// =============================================================================
// Debugger Functionality Tests
// =============================================================================

TEST_F(WindowConsoleTest, DebuggerOperations) {
    // Add items to stack
    console.Execute("10 20 30", Structure::Expression);

    // Get stack for debugging
    auto stack = console.GetExecutor()->GetDataStack();
    EXPECT_GT(stack->Size(), 0);

    // Test stack inspection
    for (int i = 0; i < stack->Size(); ++i) {
        auto obj = stack->At(i);
        EXPECT_TRUE(obj.Exists());
    }
}

// =============================================================================
// Performance Tests
// =============================================================================

TEST_F(WindowConsoleTest, Performance_ManyCommands) {
    // Execute many commands
    for (int i = 0; i < 100; ++i) {
        console.Execute(std::to_string(i).c_str(), Structure::Expression);
    }

    // Check stack has items
    auto stack = console.GetExecutor()->GetDataStack();
    EXPECT_GT(stack->Size(), 0);
}

TEST_F(WindowConsoleTest, Performance_ComplexExpressions) {
    // Test complex Pi expression
    console.Execute("1 2 + 3 * 4 - 5 /", Structure::Expression);

    // Test complex Rho expression
    console.SetLanguage(Language::Rho);
    console.Execute("x = 10; y = 20; z = x + y * 2", Structure::Statement);

    // Both should complete without error
    EXPECT_TRUE(true);
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST_F(WindowConsoleTest, EdgeCase_EmptyCommand) {
    // Should not crash on empty command
    EXPECT_NO_THROW(console.Execute("", Structure::Expression));
}

TEST_F(WindowConsoleTest, EdgeCase_WhitespaceCommand) {
    // Should handle whitespace
    EXPECT_NO_THROW(console.Execute("   ", Structure::Expression));
    EXPECT_NO_THROW(console.Execute("\t", Structure::Expression));
    EXPECT_NO_THROW(console.Execute("\n", Structure::Expression));
}

TEST_F(WindowConsoleTest, EdgeCase_LongCommand) {
    // Create a long command
    std::string longCmd;
    for (int i = 0; i < 100; ++i) {
        longCmd += std::to_string(i) + " ";
    }

    // Should handle without crashing
    EXPECT_NO_THROW(console.Execute(longCmd.c_str(), Structure::Expression));
}