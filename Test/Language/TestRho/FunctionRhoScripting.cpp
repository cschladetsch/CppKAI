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

    // CJS TODO these fail because of changes to CommonLexer
    std::cerr << "**** Skipping due to changes in Common Lexer" << std::endl;
    return;

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
    // Skip if Common Lexer issues persist
    if (_skipDueToLexerIssues) {
        std::cerr << "**** Skipping iteration tests due to Common Lexer issues" << std::endl;
        return;
    }
    
    // Create a vector of just the iteration-specific test files
    const std::vector<std::string> iterationTests = {
        "WhileLoops.rho",
        "ForLoops.rho",
        "DoWhileLoops.rho",
        "ForEachLoops.rho",
        "PropertyIteration.rho"
    };
    
    // Run each iteration test individually and report results
    for (const auto& testFile : iterationTests) {
        std::cout << "Running iteration test: " << testFile << std::endl;
        try {
            ExecScriptFile(testFile);
            std::cout << "PASSED: " << testFile << std::endl;
        }
        catch (std::exception &e) {
            std::cerr << "FAILED: " << testFile << ": " << e.what() << std::endl;
            FAIL() << "Iteration test " << testFile << " failed: " << e.what();
        }
    }
}

TEST_F(TestLangCommon, TestAdvancedIterations) {
    // Skip if Common Lexer issues persist
    if (_skipDueToLexerIssues) {
        std::cerr << "**** Skipping advanced iteration tests due to Common Lexer issues" << std::endl;
        return;
    }
    
    // Create a vector of advanced iteration test files
    const std::vector<std::string> advancedTests = {
        "AdvancedIterations.rho",
        "ErrorHandlingLoops.rho",
        "LoopOptimizations.rho"
    };
    
    // Run each advanced test individually and report results
    for (const auto& testFile : advancedTests) {
        std::cout << "Running advanced iteration test: " << testFile << std::endl;
        try {
            // Clear everything before test
            _exec->ClearStacks();
            _exec->ClearContext();
            
            // Set trace level to higher for more detailed output when testing
            int previousTraceLevel = _exec->GetTraceLevel();
            _exec->SetTraceLevel(4);
            
            // Execute the test file
            ExecScriptFile(testFile);
            std::cout << "PASSED: " << testFile << std::endl;
            
            // Restore trace level
            _exec->SetTraceLevel(previousTraceLevel);
        }
        catch (std::exception &e) {
            std::cerr << "FAILED: " << testFile << ": " << e.what() << std::endl;
            FAIL() << "Advanced iteration test failed: " << e.what();
        }
    }
}

TEST_F(TestLangCommon, TestSingleIterationScript) {
    // Skip if Common Lexer issues persist
    if (_skipDueToLexerIssues) {
        std::cerr << "**** Skipping direct iteration test due to Common Lexer issues" << std::endl;
        return;
    }
    
    // Run the direct test script
    std::cout << "Running direct iteration test: IterationTest.rho" << std::endl;
    try {
        // Clear everything before test
        _exec->ClearStacks();
        _exec->ClearContext();
        _console.GetExecutor()->SetTraceLevel(5); // Turn on tracing for more output
        
        ExecScriptFile("IterationTest.rho");
        std::cout << "PASSED: IterationTest.rho" << std::endl;
    }
    catch (std::exception &e) {
        std::cerr << "FAILED: IterationTest.rho: " << e.what() << std::endl;
        FAIL() << "Direct iteration test failed: " << e.what();
    }
}

TEST_F(TestLangCommon, TestNetworkIterations) {
    // Skip if Common Lexer issues persist
    if (_skipDueToLexerIssues) {
        std::cerr << "**** Skipping network iteration test due to Common Lexer issues" << std::endl;
        return;
    }
    
    // Run the network iteration test script
    std::cout << "Running network iteration test: NetworkIterations.rho" << std::endl;
    try {
        // Clear everything before test
        _exec->ClearStacks();
        _exec->ClearContext();
        _console.GetExecutor()->SetTraceLevel(5); // Turn on tracing for more output
        
        ExecScriptFile("NetworkIterations.rho");
        std::cout << "PASSED: NetworkIterations.rho" << std::endl;
    }
    catch (std::exception &e) {
        std::cerr << "FAILED: NetworkIterations.rho: " << e.what() << std::endl;
        FAIL() << "Network iteration test failed: " << e.what();
    }
}

KAI_END
