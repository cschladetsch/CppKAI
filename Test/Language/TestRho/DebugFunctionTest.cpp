#include <KAI/Console/Console.h>

#include "TestCommon.h"

using namespace kai;
using namespace std;

TEST(DebugFunction, SimpleAdd) {
    Console console;
    console.SetTraceLevel(10);  // Maximum tracing

    try {
        // Very simple test - define a function and call it
        console.Execute(R"(
fun add(x, y)
    return x + y

result = add(2, 3)
result
)");

        auto stack = console.GetExecutor()->GetDataStack();
        ASSERT_FALSE(stack->Empty()) << "Stack should have result";

        auto result = stack->Pop();
        if (result.IsType<int>()) {
            EXPECT_EQ(ConstDeref<int>(result), 5) << "2 + 3 should equal 5";
        } else {
            FAIL() << "Result should be an integer, got: "
                   << result.GetTypeNumber().ToInt();
        }
    } catch (const Exception& e) {
        FAIL() << "Exception: " << e.what();
    }
}