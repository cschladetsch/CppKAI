#include <gtest/gtest.h>
#include "TestLangCommon.h"

// Test suite for Tau attributes and metadata
TEST(TauAttribute, BasicAttributes) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);
    
    const char* code = R"(
        [Serializable]
        [Description("A simple point class")]
        class Point {
            [Required]
            float x;
            
            [Required]
            float y;
            
            [Computed]
            float Distance() {
                return sqrt(x * x + y * y);
            }
        }
        
        // Check if class has attributes
        Type pointType = typeof(Point);
        pointType.HasAttribute<Serializable>();
    )";
    
    console.Execute(code);
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_TRUE(kai::ConstDeref<bool>(stack->Top()));
}

TEST(TauAttribute, CustomAttributes) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);
    
    const char* code = R"(
        class ValidateRangeAttribute : Attribute {
            int min, max;
            
            ValidateRangeAttribute(int minVal, int maxVal) {
                min = minVal;
                max = maxVal;
            }
            
            bool IsValid(int value) {
                return value >= min && value <= max;
            }
        }
        
        class Player {
            [ValidateRange(0, 100)]
            int health = 100;
            
            void SetHealth(int value) {
                var attr = typeof(Player).GetField("health").GetAttribute<ValidateRange>();
                if (attr.IsValid(value)) {
                    health = value;
                }
            }
            
            int GetHealth() { return health; }
        }
        
        Player p;
        p.SetHealth(150); // Should not change
        p.GetHealth();
    )";
    
    console.Execute(code);
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 100);
}

TEST(TauAttribute, MethodAttributes) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);
    
    const char* code = R"(
        class TimingAttribute : Attribute {
            static float lastDuration;
            
            void OnMethodEnter() {
                // Start timing
            }
            
            void OnMethodExit() {
                // Stop timing and record
                lastDuration = 0.001; // Simulated
            }
        }
        
        class Calculator {
            [Timing]
            int SlowCalculation(int n) {
                int result = 0;
                for (int i = 0; i < n; i++) {
                    result += i;
                }
                return result;
            }
        }
        
        Calculator calc;
        calc.SlowCalculation(100);
        TimingAttribute.lastDuration > 0;
    )";
    
    console.Execute(code);
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_TRUE(kai::ConstDeref<bool>(stack->Top()));
}

TEST(TauAttribute, CompileTimeAttributes) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);
    
    const char* code = R"(
        [CompileTime]
        int Factorial(int n) {
            if (n <= 1) return 1;
            return n * Factorial(n - 1);
        }
        
        // This should be computed at compile time
        const int result = Factorial(5);
        result;
    )";
    
    console.Execute(code);
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 120);
}

TEST(TauAttribute, ConditionalAttributes) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);
    
    const char* code = R"(
        [Conditional("DEBUG")]
        void DebugLog(string message) {
            // This method only exists in debug builds
        }
        
        [Conditional("RELEASE")]
        void ReleaseLog(string message) {
            // This method only exists in release builds
        }
        
        // Define DEBUG
        #define DEBUG
        
        bool hasDebugLog = typeof(Program).HasMethod("DebugLog");
        hasDebugLog;
    )";
    
    console.Execute(code);
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_TRUE(kai::ConstDeref<bool>(stack->Top()));
}