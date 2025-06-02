#include <gtest/gtest.h>
#include <sstream>
#include <cmath>

#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Advanced Pi test fixture
struct PiAdvancedTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);
    }
};

// Test 1: Complex stack manipulation patterns
TEST_F(PiAdvancedTests, ComplexStackPatterns) {
    // Test rotating items deep in stack
    ExecuteScript("1 2 3 4 5 6 7 8 9 10");
    ExpectStack(10);
    
    // Rotate 5th element to top: use complex manipulation
    ExecuteScript("5 pick");
    ExpectTop(6); // 6 was at position 5 (0-indexed from top)
    
    // Test burying values
    ExecuteScript("clear 1 2 3 4 5 3 bury"); // bury 5 at position 3
    ExecuteScript("depth");
    ExpectTop(5);
}

// Test 2: Recursive factorial implementation
TEST_F(PiAdvancedTests, RecursiveFactorial) {
    // Define recursive factorial using conditionals
    ExecuteScript("{ dup 1 <= { drop 1 } { dup 1 - fact * } ife } 'fact &");
    
    // Test factorial calculations
    ExecuteScript("5 fact");
    ExpectTop(120);
    
    ExecuteScript("0 fact");
    ExpectTop(1);
    
    ExecuteScript("6 fact");
    ExpectTop(720);
}

// Test 3: Higher-order functions and combinators
TEST_F(PiAdvancedTests, HigherOrderFunctions) {
    // Y combinator for recursion
    ExecuteScript("{ dup & } 'dup& &");
    
    // Map function over a manually constructed list
    ExecuteScript("clear 1 2 3 4 5"); // Stack as list
    ExecuteScript("{ 2 * } 'double &");
    ExecuteScript("5 { double } repeat"); // Apply to each
    
    // Stack should now be 2 4 6 8 10
    ExpectStack(5);
    ExpectAt(0, 10);
    ExpectAt(1, 8);
    ExpectAt(2, 6);
}

// Test 4: Advanced control flow with nested conditions
TEST_F(PiAdvancedTests, NestedControlFlow) {
    // Implement sign function: -1 for negative, 0 for zero, 1 for positive
    ExecuteScript("{ dup 0 > { drop 1 } { dup 0 < { drop -1 } { drop 0 } ife } ife } 'sign &");
    
    ExecuteScript("5 sign");
    ExpectTop(1);
    
    ExecuteScript("-3 sign");
    ExpectTop(-1);
    
    ExecuteScript("0 sign");
    ExpectTop(0);
}

// Test 5: Stack-based Fibonacci sequence
TEST_F(PiAdvancedTests, FibonacciSequence) {
    // Generate Fibonacci numbers
    ExecuteScript("{ dup 2 < { } { dup 1 - fib swap 2 - fib + } ife } 'fib &");
    
    // Test first few Fibonacci numbers
    ExecuteScript("0 fib");
    ExpectTop(0);
    
    ExecuteScript("1 fib");
    ExpectTop(1);
    
    ExecuteScript("7 fib");
    ExpectTop(13); // 0,1,1,2,3,5,8,13
}

// Test 6: Complex string manipulation
TEST_F(PiAdvancedTests, StringManipulation) {
    // Test string concatenation and manipulation
    ExecuteScript("\"Hello\" \" \" \"World\" + +");
    ExpectString("Hello World");
    
    // Test string comparison
    ExecuteScript("\"abc\" \"abc\" ==");
    ExpectTop(true);
    
    ExecuteScript("\"abc\" \"def\" <");
    ExpectTop(true);
}

// Test 7: Prime number checker
TEST_F(PiAdvancedTests, PrimeChecker) {
    // Implement prime checking algorithm
    ExecuteScript("{ dup 2 < { drop false } { dup 2 == { drop true } { true swap dup 2 swap 1 - { dup rot dup rot % 0 == { drop drop false swap drop } { drop } ife } repeat drop } ife } ife } 'isprime &");
    
    ExecuteScript("7 isprime");
    ExpectTop(true);
    
    ExecuteScript("9 isprime");
    ExpectTop(false);
    
    ExecuteScript("2 isprime");
    ExpectTop(true);
}

// Test 8: Stack reversal algorithms
TEST_F(PiAdvancedTests, StackReversal) {
    // Reverse entire stack
    ExecuteScript("clear 1 2 3 4 5");
    ExecuteScript("depth 'n ! n @ { n @ 1 - roll } repeat");
    
    ExpectStack(5);
    ExpectAt(0, 1);
    ExpectAt(4, 5);
}

// Test 9: Accumulator patterns
TEST_F(PiAdvancedTests, AccumulatorPatterns) {
    // Sum of squares using accumulator
    ExecuteScript("clear 1 2 3 4 5");
    ExecuteScript("0 'sum ! 5 { dup * sum @ + sum ! } repeat sum @");
    
    ExpectTop(55); // 1 + 4 + 9 + 16 + 25
}

// Test 10: Custom control structures
TEST_F(PiAdvancedTests, CustomControlStructures) {
    // Implement 'times' loop
    ExecuteScript("{ 'f ! 'n ! n @ { f @ & } repeat } 'times &");
    
    // Use it to print numbers
    ExecuteScript("0 5 { 1 + dup } times drop");
    ExpectStack(5);
    ExpectAt(0, 5);
    ExpectAt(4, 1);
}

// Test 11: Error handling patterns
TEST_F(PiAdvancedTests, ErrorHandlingPatterns) {
    // Safe division that checks for zero
    ExecuteScript("{ dup 0 == { drop drop \"Division by zero\" } { / } ife } 'safediv &");
    
    ExecuteScript("10 2 safediv");
    ExpectTop(5);
    
    ExecuteScript("10 0 safediv");
    ExpectString("Division by zero");
}

// Test 12: List operations using stack
TEST_F(PiAdvancedTests, ListOperations) {
    // Implement list length counter
    ExecuteScript("clear 1 2 3 4 5");
    ExecuteScript("depth");
    ExpectTop(5);
    
    // Sum all elements
    ExecuteScript("0 swap { + } repeat");
    ExpectTop(15);
}

// Test 13: Boolean logic combinations
TEST_F(PiAdvancedTests, BooleanLogic) {
    // Test complex boolean expressions
    ExecuteScript("true true and");
    ExpectTop(true);
    
    ExecuteScript("true false or");
    ExpectTop(true);
    
    ExecuteScript("true not");
    ExpectTop(false);
    
    // XOR implementation
    ExecuteScript("{ 'b ! 'a ! a @ b @ and not a @ b @ or and } 'xor &");
    ExecuteScript("true false xor");
    ExpectTop(true);
    
    ExecuteScript("true true xor");
    ExpectTop(false);
}

// Test 14: Mathematical series
TEST_F(PiAdvancedTests, MathematicalSeries) {
    // Calculate sum of arithmetic series 1+2+3+...+n
    ExecuteScript("{ dup 1 + * 2 / } 'sumto &");
    
    ExecuteScript("10 sumto");
    ExpectTop(55);
    
    ExecuteScript("100 sumto");
    ExpectTop(5050);
}

// Test 15: Stack frame simulation
TEST_F(PiAdvancedTests, StackFrames) {
    // Simulate function call frames
    ExecuteScript("depth 'frame_start !"); // Mark frame start
    
    ExecuteScript("10 20 30"); // Local variables
    ExecuteScript("depth frame_start @ -"); // Calculate frame size
    ExpectTop(3);
    
    // Clean up frame
    ExecuteScript("{ drop } repeat");
}

// Test 16: Type checking functions
TEST_F(PiAdvancedTests, TypeChecking) {
    // Create type checking predicates
    ExecuteScript("42 dup type");
    ExpectString("int");
    
    ExecuteScript("3.14 dup type");
    ExpectString("float");
    
    ExecuteScript("\"hello\" dup type");
    ExpectString("string");
}

// Test 17: Memoization pattern
TEST_F(PiAdvancedTests, Memoization) {
    // Simple memoization for expensive calculations
    ExecuteScript("{ } 'memo_table &");
    
    // Memoized function wrapper
    ExecuteScript("{ 'fn ! 'key ! memo_table @ key @ has { memo_table @ key @ @ } { key @ fn @ & dup memo_table @ key @ rot ! } ife } 'memoize &");
    
    // Use with factorial
    ExecuteScript("{ dup 1 <= { drop 1 } { dup 1 - fact_memo * } ife } 'fact_base &");
    ExecuteScript("{ fact_base memoize } 'fact_memo &");
}

// Test 18: Continuation patterns
TEST_F(PiAdvancedTests, ContinuationPatterns) {
    // Save and restore continuations
    ExecuteScript("mark"); // Mark current position
    ExecuteScript("1 2 3");
    ExecuteScript("depth 'saved_depth !");
    
    ExecuteScript("clear");
    ExecuteScript("saved_depth @ { 0 } repeat"); // Restore depth
    ExpectStack(3);
}

// Test 19: Advanced arithmetic patterns
TEST_F(PiAdvancedTests, AdvancedArithmetic) {
    // Implement power function
    ExecuteScript("{ 'exp ! 'base ! 1 exp @ { base @ * } repeat } 'pow &");
    
    ExecuteScript("2 10 pow");
    ExpectTop(1024);
    
    ExecuteScript("3 4 pow");
    ExpectTop(81);
    
    // Integer square root
    ExecuteScript("{ 1 { dup dup * 3 pick > { 1 + } { drop swap drop leave } ife } loop } 'isqrt &");
    
    ExecuteScript("16 isqrt");
    ExpectTop(4);
    
    ExecuteScript("10 isqrt");
    ExpectTop(3); // Floor of sqrt(10)
}

// Test 20: Complex data structure simulation
TEST_F(PiAdvancedTests, DataStructureSimulation) {
    // Simulate a simple record/struct using naming
    ExecuteScript("100 'person.age !");
    ExecuteScript("\"John\" 'person.name !");
    ExecuteScript("true 'person.active !");
    
    // Access fields
    ExecuteScript("person.age @");
    ExpectTop(100);
    
    ExecuteScript("person.name @");
    ExpectString("John");
    
    ExecuteScript("person.active @");
    ExpectTop(true);
    
    // Update field
    ExecuteScript("person.age @ 1 + person.age !");
    ExecuteScript("person.age @");
    ExpectTop(101);
}