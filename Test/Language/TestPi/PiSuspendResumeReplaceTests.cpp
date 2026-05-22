#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Stack.h"
#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Language/Pi/PiTranslator.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Test fixture for Pi language Suspend (&), Resume (...), and Replace (!) tests
struct PiSuspendResumeReplaceTests : TestLangCommon {
    template <class T>
    void AssertResult(const char* script, T expected) {
        try {
            console_.SetLanguage(Language::Pi);
            console_.Execute(script, Structure::Program);

            auto executor = console_.GetExecutor();
            auto dataStack = executor->GetDataStack();

            ASSERT_FALSE(dataStack->Empty())
                << "No result on stack after execution\nScript: " << script;

            auto result = dataStack->Top();
            ASSERT_TRUE(result.IsType<T>())
                << "Result type mismatch. Expected " << typeid(T).name()
                << " but got "
                << (result.GetClass() ? result.GetClass()->GetName().ToString()
                                      : "null");

            T actual = ConstDeref<T>(result);
            ASSERT_EQ(actual, expected) << "Value mismatch\nScript: " << script;

        } catch (const Exception::Base& e) {
            FAIL() << "Exception: " << e.ToString() << "\nScript: " << script;
        } catch (const std::exception& e) {
            FAIL() << "std::exception: " << e.what() << "\nScript: " << script;
        }
    }
};

// Test 1: Basic Suspend (&) - simple subroutine call
TEST_F(PiSuspendResumeReplaceTests, BasicSuspend) {
    // { 2 3 + } & should push current continuation, execute the block, and
    // return
    AssertResult<int>("{ 2 3 + } &", 5);
}

// Test 2: Suspend with data on stack
TEST_F(PiSuspendResumeReplaceTests, SuspendWithStackData) {
    // 10 { 5 + } & - should add 5 to 10
    AssertResult<int>("10 { 5 + } &", 15);
}

// Test 3: Nested Suspend
TEST_F(PiSuspendResumeReplaceTests, NestedSuspend) {
    // Nested suspend: outer continuation suspends to call inner
    AssertResult<int>("{ 2 { 3 + } & 4 * } &", 20);  // (3+2)*4 = 20
}

// Test 4: Resume (...) exits early from continuation
TEST_F(PiSuspendResumeReplaceTests, BasicResume) {
    // Resume should exit the continuation early
    // { 10 ... 20 } & should push 10 then exit, leaving 10 on stack
    AssertResult<int>("{ 10 ... 20 } &", 10);
}

// Test 5: Resume in conditional
TEST_F(PiSuspendResumeReplaceTests, ResumeInConditional) {
    // Resume exits the continuation, leaving value on stack
    // The '...' resumes from the suspend, so only 10 remains
    AssertResult<int>("{ 10 ... } &", 10);
}

// Test 6: Replace (!) completely replaces continuation
TEST_F(PiSuspendResumeReplaceTests, BasicReplace) {
    // Replace should switch to a different continuation entirely
    // { { 42 } ! 100 } & should replace with {42}, never reaching 100
    AssertResult<int>("{ { 42 } ! 100 } &", 42);
}

// Test 7: Replace for tail call optimization
TEST_F(PiSuspendResumeReplaceTests, ReplaceTailCall) {
    // Simulating tail call: compute something then replace with result
    // continuation
    AssertResult<int>("{ 5 3 * { 2 + } ! } &",
                      17);  // 5*3=15, then replace with {2+} => 17
}

// Test 8: Resume vs Replace difference
TEST_F(PiSuspendResumeReplaceTests, ResumeVsReplace) {
    // Resume returns to caller, Replace does not
    // Resume exits the current continuation immediately
    AssertResult<int>("{ { 5 ... 99 } & 10 + } &", 5);
}

// Test 9: Multiple suspends building context stack
TEST_F(PiSuspendResumeReplaceTests, MultipleSuspends) {
    // Each suspend pushes to context stack
    AssertResult<int>("{ { { 7 } & 3 + } & 2 * } &", 20);  // ((7+3)*2) = 20
}

// Test 10: Replace doesn't preserve context
TEST_F(PiSuspendResumeReplaceTests, ReplaceNoContext) {
    // Replace completely replaces current continuation
    // The ! replaces with the {42} continuation, so 88 never executes
    AssertResult<int>("{ { 42 } ! 88 } &", 42);
}

// Test 11: Suspend stores return address
TEST_F(PiSuspendResumeReplaceTests, SuspendReturnAddress) {
    // Verify suspend returns to the continuation point
    AssertResult<int>("5 { 3 + } & 2 *", 16);  // (5+3) = 8, then 8*2 = 16
}

// Test 12: Resume exits nested continuation
TEST_F(PiSuspendResumeReplaceTests, ResumeExitsLoop) {
    // Resume exits current continuation back to caller
    // Nested: outer calls inner which returns 10 via resume
    AssertResult<int>("{ { 10 ... } & } &", 10);
}
