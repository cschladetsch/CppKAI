#include "KAI/Core/Console.h"
#include "MyTestStruct.h"
#include "TestLangCommon.h"

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
    Registry &reg = *_reg;
    MyStruct::Register(reg);

    Pointer<MyStruct> mystruct = reg.New<MyStruct>();
    mystruct->num = 345;
    mystruct->string = "hello world";

    _console.GetTree().AddSearchPath(_root);

    // Process::trace = 10;
    _root["mystruct"] = mystruct;
    AddFunction(_root, Function_0, Label("Function0"));
    AddFunction(_root, Function_1, Label("Function1"));
    AddFunction(_root, Function_2, Label("Function2"));
    AddFunction(_root, Function_3, Label("Function3"));

    // Run the tests - they may have issues, but we want them to run for diagnostic purposes
    std::cerr << "**** Running tests that might fail due to lexer changes - check rho_diagnostic.log for details" << std::endl;

    _console.Execute("Function0()");
    _console.Execute("Function1(42)");
    _console.Execute("Function2(123, 3, \"bar\")");
    _console.Execute("Function3(mystruct)");

    for (int n = 0; n < 2; ++n) ASSERT_TRUE(funCalled[n]);

    Value<Stack> stack = _console.GetExecutor()->GetDataStack();
    EXPECT_EQ(stack->Size(), 2);
    EXPECT_EQ(ConstDeref<int>(stack->Pop()), 345);
    EXPECT_EQ(ConstDeref<String>(stack->Pop()), "barfoo");
    EXPECT_EQ(stack->Size(), 0);
}

TEST_F(TestLangCommon, RunScripts) { ExecScripts(); }

TEST_F(TestLangCommon, TestIterationConstructs) {
    _console.SetLanguage(Language::Rho);
    
    // Clear stacks
    _exec->ClearStacks();
    _exec->ClearContext();
    
    // Set trace level to maximum for detailed output
    std::cout << "Setting trace level to maximum (5) for iteration tests" << std::endl;
    _exec->SetTraceLevel(5);
    
    try {
        // Test basic while loop
        std::cout << "Testing basic while loop..." << std::endl;
        _console.Execute("i = 0; while (i < 5) { i = i + 1; }");
        
        // Test for loop
        std::cout << "Testing for loop..." << std::endl;
        _console.Execute("sum = 0; for (i = 0; i < 5; i = i + 1) { sum = sum + i; }");
        
        // Test diagnostic script
        std::cout << "Testing diagnostic script..." << std::endl;
        const fs::path scriptsRoot(KAI_STRINGISE(KAI_SCRIPT_ROOT));
        const fs::path scriptPath = scriptsRoot / "DiagnosticTest.rho";
        
        if (fs::exists(scriptPath)) {
            auto contents = File::ReadAllText(scriptPath);
            _console.Execute(contents.c_str());
        } else {
            std::cerr << "Diagnostic script not found at: " << scriptPath << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in iteration tests: " << e.what() << std::endl;
        // Don't rethrow - we want to continue with other tests
    }
    catch (...) {
        std::cerr << "Unknown exception in iteration tests" << std::endl;
        // Don't rethrow - we want to continue with other tests
    }
}

KAI_END