#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "KAI/Core/BuiltinTypes/Stack.h"
#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Language/Pi/PiTranslator.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Test fixture for Pi language continuation tests
struct PiContinuationTests : TestLangCommon {
    // Helper method to execute a Pi script and verify the result
    template <class T>
    void AssertResult(const char *script, T expected, bool verbose = false) {
        if (verbose) {
            KAI_LOG_INFO(std::string("Testing script: ") + script);
        }

        try {
            Registry reg;
            Console console(reg);
            console.SetScope(reg.GetGlobalScope());

            auto result = console.Execute(script);

            if (result.Failed) {
                if (verbose) {
                    KAI_LOG_ERROR("Execution failed: " + result.Error);
                }
                FAIL() << "Error executing script: " << result.Error;
                return;
            }

            if (console.GetExecutor().GetStack().Size() == 0) {
                if (verbose) {
                    KAI_LOG_WARNING("Stack is empty, cannot verify result");
                }
                FAIL() << "Stack is empty, cannot verify result";
                return;
            }

            auto val = console.GetExecutor().GetStack().Top();
            if (val.GetType() != Type::Traits<T>::TypeNumber) {
                if (verbose) {
                    KAI_LOG_ERROR("Type mismatch. Expected: " +
                                  std::to_string(Type::Traits<T>::TypeNumber) +
                                  ", Got: " + std::to_string(val.GetType()));
                }
                FAIL() << "Type mismatch. Expected: "
                       << Type::Traits<T>::TypeNumber
                       << ", Got: " << val.GetType();
                return;
            }

            T actual = kai_cast<T>(val);
            if (verbose) {
                KAI_LOG_INFO("Result: " + std::to_string(actual));
            }
            ASSERT_EQ(expected, actual)
                << "Result doesn't match expected value";
        } catch (const Exception &e) {
            if (verbose) {
                KAI_LOG_ERROR("Exception: " + std::string(e.ToString()));
            }
            FAIL() << "Exception: " << e.ToString();
        } catch (const std::exception &e) {
            if (verbose) {
                KAI_LOG_ERROR("std::exception: " + std::string(e.what()));
            }
            FAIL() << "std::exception: " << e.what();
        } catch (...) {
            if (verbose) {
                KAI_LOG_ERROR("Unknown exception");
            }
            FAIL() << "Unknown exception";
        }
    }
};

// Test basic quotation (code blocks)
TEST_F(PiContinuationTests, BasicQuotation) {
    // Creating a quotation
    AssertResult<int>("{ 2 3 + } call", 5);

    // Quotation with stack operations
    AssertResult<int>("5 { 2 * } call", 10);
    AssertResult<int>("5 10 { + } call", 15);
    AssertResult<int>("5 10 { swap - } call", 5);

    // Nested quotations
    AssertResult<int>("5 { 2 { 3 + } call * } call", 20);
}

// Test quotation as data
TEST_F(PiContinuationTests, QuotationAsData) {
    // Store and retrieve a quotation
    AssertResult<int>("{ 2 3 + } 'q' ! 'q' @ call", 5);

    // Pass quotation as parameter
    AssertResult<int>("{ 2 * } 10 swap call", 20);

    // Manipulate multiple quotations
    AssertResult<int>("{ 2 + } { 3 * } 5 rot call swap call", 21);
}

// Test conditional execution with quotations
TEST_F(PiContinuationTests, ConditionalExecution) {
    // If-then with quotations
    AssertResult<int>("5 true { 2 * } if", 10);
    AssertResult<int>("5 false { 2 * } if", 5);

    // If-then-else with quotations
    AssertResult<int>("5 true { 2 * } { 2 + } ife", 10);
    AssertResult<int>("5 false { 2 * } { 2 + } ife", 7);

    // Dynamic conditions
    AssertResult<int>("5 7 < { 10 } { 20 } ife", 10);
    AssertResult<int>("5 3 < { 10 } { 20 } ife", 20);
}

// Test loop control with quotations
TEST_F(PiContinuationTests, LoopControl) {
    // While loop (sum from 1 to 5)
    AssertResult<int>(
        "0 'sum' ! "
        "1 'i' ! "
        "{ 'i' @ 'sum' +! "
        "  'i' @ 1 + 'i' ! "
        "  'i' @ 5 <= "
        "} "
        "while "
        "'sum' @",
        15);

    // Until loop (sum from 1 to 5)
    AssertResult<int>(
        "0 'sum' ! "
        "1 'i' ! "
        "{ 'i' @ 'sum' +! "
        "  'i' @ 1 + 'i' ! "
        "  'i' @ 5 > "
        "} "
        "until "
        "'sum' @",
        15);

    // Do loop (sum from 1 to 5) - Pi implementation of a counted loop
    AssertResult<int>(
        "0 'sum' ! "
        "1 5 1 "
        "{ 'i' ! "  // loop variable
        "  'i' @ 'sum' +! "
        "} "
        "for "
        "'sum' @",
        15);
}

// Test with stack continuations
TEST_F(PiContinuationTests, StackContinuations) {
    // Save and restore stack
    AssertResult<int>(
        "1 2 3 "
        "save-stack 'saved' ! "
        "drop drop drop "
        "42 "
        "'saved' @ restore-stack "
        "drop drop",
        1);
}

// Test with custom combinators
TEST_F(PiContinuationTests, CustomCombinators) {
    // Define a custom 'dip' combinator that applies a quotation to the second
    // element
    AssertResult<int>(
        "{ 'fn' ! "       // store the quotation
        "  swap "         // bring second element to top
        "  'fn' @ call "  // apply quotation
        "  swap "         // restore order
        "} 'dip' ! "      // store as 'dip'

        "5 10 { 2 * } 'dip' @ call",  // apply dip to multiply 5 by 2, with 10
                                      // on top
        10);  // top of stack is still 10, but 5 became 10

    // Define a custom 'keep' combinator that applies a quotation but keeps the
    // original
    AssertResult<int>(
        "{ 'fn' ! "       // store the quotation
        "  over "         // duplicate second element
        "  'fn' @ call "  // apply quotation to the copy
        "} 'keep' ! "     // store as 'keep'

        "5 { 2 * } 'keep' @ call +",  // apply keep to multiply 5 by 2, then add
                                      // to original 5
        15);                          // result is 5 + 10 = 15
}

// Test exception handling patterns
TEST_F(PiContinuationTests, ExceptionHandling) {
    // Simple try-catch pattern
    AssertResult<int>(
        "{ "
        "  10 0 / "  // division by zero (would cause error)
        "} "
        "{ "
        "  drop 0 "  // handle error, return 0
        "} "
        "catch",
        0);
}

// Test recursion with continuations
TEST_F(PiContinuationTests, RecursiveContinuations) {
    // Factorial with self-calling quotation
    AssertResult<int>(
        "{ 'factorial' ! "                     // name this quotation
        "  dup 0 <= "                          // base case
        "  { drop 1 } "                        // return 1 for n <= 0
        "  { dup 1 - 'factorial' @ call * } "  // recursive case: n *
                                               // factorial(n-1)
        "  ife "
        "} 'factorial' ! "  // store the quotation

        "5 'factorial' @ call",  // calculate factorial(5)
        120);

    // Fibonacci with self-calling quotation
    AssertResult<int>(
        "{ 'fibonacci' ! "                  // name this quotation
        "  dup 2 < "                        // base case
        "  { } "                            // return n for n < 2
        "  { dup 1 - 'fibonacci' @ call "   // fibonacci(n-1)
        "    swap 2 - 'fibonacci' @ call "  // fibonacci(n-2)
        "    + "                            // add results
        "  } "
        "  ife "
        "} 'fibonacci' ! "  // store the quotation

        "7 'fibonacci' @ call",  // calculate fibonacci(7)
        13);
}

// Test with continuation composition
TEST_F(PiContinuationTests, ContinuationComposition) {
    // Create function composition
    AssertResult<int>(
        "{ 2 * } 'double' ! "     // function to double a number
        "{ 1 + } 'increment' ! "  // function to increment a number

        "{ 'g' ! 'f' ! "  // compose: first g, then f
        "  'g' @ call 'f' @ call "
        "} 'compose' ! "

        "5 'double' @ 'increment' @ 'compose' @ call",  // (5*2)+1
        11);

    // More complex composition with multiple data items
    AssertResult<int>(
        "{ + } 'add' ! "       // function to add two numbers
        "{ * } 'multiply' ! "  // function to multiply two numbers

        "3 4 'add' @ call "     // 3+4 = 7
        "5 'multiply' @ call",  // 7*5 = 35
        35);
}

// Test with continuation customization
TEST_F(PiContinuationTests, ContinuationCustomization) {
    // Create a partial application
    AssertResult<int>(
        "{ 'n' ! "        // take a number parameter
        "  { 'n' @ + } "  // return a function that adds n
        "} 'adder' ! "

        "5 'adder' @ call "  // create function that adds 5
        "'add5' ! "

        "10 'add5' @ call",  // apply: 10+5
        15);

    // Create a function that processes a list of operations
    AssertResult<int>(
        "{ 'ops' ! 'val' ! "        // take initial value and list of operations
        "  'ops' @ "                // get the operations list
        "  { 'op' ! "               // for each operation
        "    'val' @ 'op' @ call "  // apply it to the value
        "    'val' ! "              // update the value
        "  } "
        "  foreach "  // apply for each element
        "  'val' @ "  // return final value
        "} 'process' ! "

        "5 "                            // initial value
        "[ { 2 * } { 3 + } { 2 / } ] "  // list of operations
        "'process' @ call",  // apply: ((5*2)+3)/2 = 6.5 -> 6 (integer division)
        6);
}

// Test with continuation-based state machines
TEST_F(PiContinuationTests, StateMachines) {
    // Simple state machine
    AssertResult<int>(
        "1 'state' ! "  // initial state

        "{ 'arg' ! 'fn' ! "           // take function and argument
        "  'state' @ "                // get current state
        "  { "                        // state 1 handler
        "    'arg' @ 2 * 'state' ! "  // update state to arg*2
        "    2 'state' ! "            // set next state
        "  } "
        "  { "                        // state 2 handler
        "    'arg' @ 3 + 'state' ! "  // update state to arg+3
        "    1 'state' ! "            // set next state
        "  } "
        "  'state' @ 1 - pick "  // select handler based on state (0-indexed)
        "  'fn' @ call "         // call the handler
        "} 'process-state' ! "

        "{ } 5 'process-state' @ call "  // initial state is 1, so 5*2 = 10, new
                                         // state is 2
        "{ } 'process-state' @ call "  // state is 2, so 10+3 = 13, new state is
                                       // 1
        "'state' @",                   // final state value
        13);
}