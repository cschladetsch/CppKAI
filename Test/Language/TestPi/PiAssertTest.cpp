#include <KAI/Core/BuiltinTypes.h>

#include "TestLangCommon.h"

using namespace kai;
using namespace std;

struct PiAssertTest : TestLangCommon {};

TEST_F(PiAssertTest, TestSimpleAssert) {
    // Set language to Pi
    console_.SetLanguage(Language::Pi);

    // Clear data stack
    data_->Clear();

    // Execute the Pi code "1 1 + 2 assert"
    // This should execute: push 1, push 1, add them (result 2), push 2, assert
    // 2 == 2
    console_.Execute("1 1 + 2 assert");

    // If we get here, the assertion passed
    SUCCEED() << "Assertion passed successfully";
}