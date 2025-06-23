#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct MapIndexingTest : TestLangCommon {};

TEST_F(MapIndexingTest, MapWithDoubleQuotes) {
    console_.SetLanguage(Language::Rho);
    const char* code = R"(
map = {}
map["one"] = 1
map["two"] = 2
map["three"] = 3
value = map["two"]
value
)";

    console_.Execute(code, Structure::Program);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(ConstDeref<int>(result), 2);
}