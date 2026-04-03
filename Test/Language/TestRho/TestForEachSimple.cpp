#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct TestForEachSimple : TestLangCommon {};

TEST_F(TestForEachSimple, SingleForEach) {
    console_.SetLanguage(Language::Rho);

    const char* code = "foreach x in [1]\n    x";

    console_.Execute(code, Structure::Program);

    // Just check we got something
    ASSERT_FALSE(data_->Empty());
}