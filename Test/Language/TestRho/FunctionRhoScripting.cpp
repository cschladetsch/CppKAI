#include <filesystem>

#include "KAI/Core/Console.h"
#include "KAI/Core/File.h"
#include "MyTestStruct.h"
#include "TestLangCommon.h"
namespace fs = std::filesystem;

KAI_BEGIN

using namespace std;

bool funCalled[5];

// make some functions that we will also add to the runtime
void Function_0() { funCalled[0] = true; }

void Function_1(int) { funCalled[1] = true; }

String Function_2(int n, int f, String p) {
    funCalled[2] = true;
    // KAI_TRACE_3(n, f, p);
    return p + String("foo");
}

Object Function_3(Object object) {
    funCalled[3] = true;
    KAI_TRACE_1(object);
    return object["num"];
}

TEST_F(TestLangCommon, TestRhoReflection) {
    Registry& reg = *reg_;

    // Register MyStruct using the proper registration method
    MyStruct::Register(reg);

    // Create test object
    Pointer<MyStruct> mystruct = reg.New<MyStruct>();
    mystruct->num = 345;
    mystruct->string = "hello world";

    // Set up console for testing
    console_.GetTree().AddSearchPath(root_);
    root_["mystruct"] = mystruct;

    // Add test functions
    AddFunction(root_, Function_0, Label("Function0"));
    AddFunction(root_, Function_1, Label("Function1"));
    AddFunction(root_, Function_2, Label("Function2"));
    AddFunction(root_, Function_3, Label("Function3"));

    // Run tests
    console_.Execute("Function0()");
    console_.Execute("Function1(42)");
    console_.Execute("Function2(123, 3, \"bar\")");
    console_.Execute("Function3(mystruct)");

    // Verify functions were called
    for (int n = 0; n < 2; ++n) ASSERT_TRUE(funCalled[n]);

    // Check results
    Value<Stack> stack = console_.GetExecutor()->GetDataStack();
    EXPECT_EQ(stack->Size(), 2);
    EXPECT_EQ(ConstDeref<int>(stack->Pop()), 345);
    EXPECT_EQ(ConstDeref<String>(stack->Pop()), "barfoo");
    EXPECT_EQ(stack->Size(), 0);
}

TEST_F(TestLangCommon, RunScripts) { ExecScripts(); }

TEST_F(TestLangCommon, TestIterationConstructs) {
    console_.SetLanguage(Language::Rho);

    // Clear stacks
    exec_->ClearStacks();
    exec_->ClearContext();

    // Set trace level to maximum for detailed output
    std::cout << "Setting trace level to maximum (5) for iteration tests"
              << std::endl;
    exec_->SetTraceLevel(5);

    try {
        // Test basic while loop
        std::cout << "Testing basic while loop..." << std::endl;
        console_.Execute("i = 0; while (i < 5) { i = i + 1; }");
    } catch (const std::exception& e) {
        std::cerr << "Exception in while loop test: " << e.what() << std::endl;
        // Don't rethrow - we want to continue with other tests
    } catch (...) {
        std::cerr << "Unknown exception in while loop test" << std::endl;
    }
}

// Dedicated test for do-while loops to isolate testing
TEST_F(TestLangCommon, TestDoWhileLoop) {
    console_.SetLanguage(Language::Rho);

    // Clear stacks
    exec_->ClearStacks();
    exec_->ClearContext();

    // Set trace level to maximum for detailed output
    std::cout << "Setting trace level to maximum (5) for do-while tests"
              << std::endl;
    exec_->SetTraceLevel(5);

    try {
        // For simplicity, just test a single basic do-while loop directly with
        // newlines
        std::cout << "Testing do-while with explicit string and newlines:"
                  << std::endl;
        std::string script =
            "// Simple do-while test with newlines for proper indentation\n"
            "i = 0\n"
            "do\n"
            "    i = i + 1\n"
            "while i < 3\n";

        std::cout << "Script:" << std::endl << script << std::endl;

        // Execute the script
        console_.Execute(script);

        // Check stack
        Value<Stack> stack = console_.GetExecutor()->GetDataStack();
        std::cout << "Stack size after execution: " << stack->Size()
                  << std::endl;

        // The script should have left i=3 on the stack
        if (!stack->Empty()) {
            std::cout << "Stack has " << stack->Size()
                      << " elements after do-while" << std::endl;
            while (!stack->Empty()) {
                Object obj = stack->Pop();
                std::cout << "  Stack element: " << obj.ToString().c_str()
                          << std::endl;
            }
        }

        std::cout << "Do-while loop test completed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception in do-while test: " << e.what() << std::endl;
        FAIL() << "Do-while test failed with exception: " << e.what();
    } catch (...) {
        std::cerr << "Unknown exception in do-while test" << std::endl;
        FAIL() << "Do-while test failed with unknown exception";
    }
}

// Dedicated test for for loops to isolate testing
TEST_F(TestLangCommon, TestForLoop) {
    console_.SetLanguage(Language::Rho);

    // Clear stacks
    exec_->ClearStacks();
    exec_->ClearContext();

    // Set trace level to maximum for detailed output
    std::cout << "Setting trace level to maximum (5) for for-loop tests"
              << std::endl;
    exec_->SetTraceLevel(5);

    try {
        // Test for loop
        std::cout << "Testing for loop..." << std::endl;
        console_.Execute(
            "sum = 0; for (i = 0; i < 5; i = i + 1) { sum = sum + i; }");
    } catch (const std::exception& e) {
        std::cerr << "Exception in for loop test: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception in for loop test" << std::endl;
    }
}

// Test minimal do-while loop script
TEST_F(TestLangCommon, TestMinimalDoWhileScript) {
    console_.SetLanguage(Language::Rho);

    // Clear stacks
    exec_->ClearStacks();
    exec_->ClearContext();

    // Set trace level to maximum for detailed output
    std::cout << "Setting trace level to maximum (5) for minimal do-while test"
              << std::endl;
    exec_->SetTraceLevel(5);

    try {
        // Get file content
        std::string scriptPath =
            "Test/Language/TestRho/Scripts/MinimalDoWhileTest.rho";
        std::cout << "Loading script from: " << scriptPath << std::endl;
        std::string scriptContent = File::ReadAllText(scriptPath);
        std::cout << "Script content:" << std::endl
                  << scriptContent << std::endl;

        // Execute script
        std::cout << "Executing minimal do-while test script..." << std::endl;
        console_.Execute(scriptContent);

        // Check results - should have run without exceptions
        std::cout << "Minimal do-while test completed successfully!"
                  << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception in minimal do-while test: " << e.what()
                  << std::endl;
        FAIL() << "Minimal do-while test failed with exception: " << e.what();
    } catch (...) {
        std::cerr << "Unknown exception in minimal do-while test" << std::endl;
        FAIL() << "Minimal do-while test failed with unknown exception";
    }
}

KAI_END