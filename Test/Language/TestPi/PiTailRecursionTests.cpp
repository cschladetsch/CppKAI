#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Stack.h"
#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Language/Pi/PiTranslator.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Test fixture for Pi tail recursion using ! (Replace)
struct PiTailRecursionTests : TestLangCommon {
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

// Simple tail recursion: countdown
TEST_F(PiTailRecursionTests, CountdownZero) {
    AssertResult<int>(
        "{ dup 0 == { } { 1 - countdown ! } ife } 'countdown # 0 countdown &",
        0);
}

TEST_F(PiTailRecursionTests, CountdownOne) {
    AssertResult<int>(
        "{ dup 0 == { } { 1 - countdown ! } ife } 'countdown # 1 countdown &",
        0);
}

TEST_F(PiTailRecursionTests, CountdownFive) {
    AssertResult<int>(
        "{ dup 0 == { } { 1 - countdown ! } ife } 'countdown # 5 countdown &",
        0);
}

TEST_F(PiTailRecursionTests, CountdownTen) {
    AssertResult<int>(
        "{ dup 0 == { } { 1 - countdown ! } ife } 'countdown # 10 countdown &",
        0);
}

// Tail recursive sum with accumulator
TEST_F(PiTailRecursionTests, TailSumZero) {
    // sum(0, 0) = 0
    AssertResult<int>(
        "{ swap dup 0 == { drop } { swap over + swap 1 - sum ! } ife } 'sum # "
        "0 0 sum &",
        0);
}

TEST_F(PiTailRecursionTests, TailSumFive) {
    // sum(5, 0) = 1+2+3+4+5 = 15
    // Stack starts as [n, acc], we compute sum(n-1, acc+n)
    AssertResult<int>(
        "{ swap dup 0 == { drop } { swap over + swap 1 - swap sum ! } ife } "
        "'sum # 5 0 sum &",
        15);
}

TEST_F(PiTailRecursionTests, TailSumTen) {
    // sum(10, 0) = 1+2+...+10 = 55
    // Stack starts as [n, acc], we compute sum(n-1, acc+n)
    AssertResult<int>(
        "{ swap dup 0 == { drop } { swap over + swap 1 - swap sum ! } ife } "
        "'sum # 10 0 sum &",
        55);
}

// Tail recursive factorial
TEST_F(PiTailRecursionTests, TailFactorialFive) {
    // factorial(5, 1) = 120
    // Stack starts as [n, acc], we compute fact(n-1, acc*n)
    AssertResult<int>(
        "{ swap dup 0 == { drop } { swap over * swap 1 - swap fact ! } ife } "
        "'fact # 5 1 fact &",
        120);
}

// Tail recursive even/odd check
TEST_F(PiTailRecursionTests, IsEvenZero) {
    AssertResult<bool>(
        "{ dup 0 == { drop true } { dup 1 == { drop false } { 2 - isEven ! } "
        "ife } ife } 'isEven # 0 isEven &",
        true);
}

TEST_F(PiTailRecursionTests, IsEvenOne) {
    AssertResult<bool>(
        "{ dup 0 == { drop true } { dup 1 == { drop false } { 2 - isEven ! } "
        "ife } ife } 'isEven # 1 isEven &",
        false);
}

TEST_F(PiTailRecursionTests, IsEvenEight) {
    AssertResult<bool>(
        "{ dup 0 == { drop true } { dup 1 == { drop false } { 2 - isEven ! } "
        "ife } ife } 'isEven # 8 isEven &",
        true);
}

TEST_F(PiTailRecursionTests, IsOddSeven) {
    AssertResult<bool>(
        "{ dup 0 == { drop false } { dup 1 == { drop true } { 2 - isOdd ! } "
        "ife } ife } 'isOdd # 7 isOdd &",
        true);
}
