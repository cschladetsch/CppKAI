#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct SimplestForEachTest : TestLangCommon {};

TEST_F(SimplestForEachTest, JustForEach) {
    console_.SetLanguage(Language::Rho);

    // Simplest possible foreach - doesn't actually do anything
    const char* code = R"(
foreach x in [1]
    x
)";

    console_.Execute(code, Structure::Program);

    // Even if foreach fails, there should be something on the stack
    ASSERT_FALSE(data_->Empty());
}