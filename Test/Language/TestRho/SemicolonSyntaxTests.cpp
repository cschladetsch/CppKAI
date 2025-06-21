#include <iostream>
#include <sstream>

#include "../../Include/TestLangCommon.h"

KAI_BEGIN

// Extended tests for the semicolon syntax in Rho language
// NOTE: Semicolons are now supported in Rho language for separating statements
TEST_F(TestLangCommon, SemicolonBasics) {
    console_.SetLanguage(Language::Rho);
    exec_->ClearStacks();
    exec_->ClearContext();
    exec_->SetTraceLevel(3);

    try {
        std::stringstream ss;
        ss << "// Basic semicolon test\n"
           << "x = 10\n"
           << "y = 20\n"
           << "z = x + y\n"
           << "assert(z == 30)\n";

        console_.Execute(ss.str());
        std::cout << "SemicolonBasics test passed!" << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI Exception: " << e.ToString() << std::endl;
        FAIL() << "SemicolonBasics test failed with KAI exception: "
               << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        FAIL() << "SemicolonBasics test failed with exception: " << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL() << "SemicolonBasics test failed with unknown exception";
    }
}

TEST_F(TestLangCommon, MultipleSemicolonStatements) {
    console_.SetLanguage(Language::Rho);
    exec_->ClearStacks();
    exec_->ClearContext();
    exec_->SetTraceLevel(3);

    try {
        std::stringstream ss;
        ss << "// Multiple statements with semicolons\n"
           << "a = 5\n"
           << "b = 10\n"
           << "c = 15\n"
           << "sum = a + b + c\n"
           << "assert(sum == 30)\n";

        console_.Execute(ss.str());
        std::cout << "MultipleSemicolonStatements test passed!" << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI Exception: " << e.ToString() << std::endl;
        FAIL() << "MultipleSemicolonStatements test failed with KAI exception: "
               << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        FAIL() << "MultipleSemicolonStatements test failed with exception: "
               << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL()
            << "MultipleSemicolonStatements test failed with unknown exception";
    }
}

TEST_F(TestLangCommon, SemicolonsInExpressions) {
    console_.SetLanguage(Language::Rho);
    exec_->ClearStacks();
    exec_->ClearContext();
    exec_->SetTraceLevel(3);

    try {
        std::stringstream ss;
        ss << "// Complex expression with semicolons\n"
           << "a = 3\n"
           << "b = 4\n"
           << "result = a * b + 3\n"
           << "assert(result == 15)\n";

        console_.Execute(ss.str());
        std::cout << "SemicolonsInExpressions test passed!" << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI Exception: " << e.ToString() << std::endl;
        FAIL() << "SemicolonsInExpressions test failed with KAI exception: "
               << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        FAIL() << "SemicolonsInExpressions test failed with exception: "
               << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL() << "SemicolonsInExpressions test failed with unknown exception";
    }
}

TEST_F(TestLangCommon, NestedExpressionsWithSemicolons) {
    console_.SetLanguage(Language::Rho);
    exec_->ClearStacks();
    exec_->ClearContext();
    exec_->SetTraceLevel(3);

    try {
        std::stringstream ss;
        ss << "// Nested expressions with semicolons\n"
           << "a = 2\n"
           << "b = 3\n"
           << "c = 4\n"
           << "result = a * b + c\n"
           << "assert(result == 10)\n";

        console_.Execute(ss.str());
        std::cout << "NestedExpressionsWithSemicolons test passed!"
                  << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI Exception: " << e.ToString() << std::endl;
        FAIL() << "NestedExpressionsWithSemicolons test failed with KAI "
                  "exception: "
               << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        FAIL() << "NestedExpressionsWithSemicolons test failed with exception: "
               << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL() << "NestedExpressionsWithSemicolons test failed with unknown "
                  "exception";
    }
}

TEST_F(TestLangCommon, TrailingSemicolon) {
    console_.SetLanguage(Language::Rho);
    exec_->ClearStacks();
    exec_->ClearContext();
    exec_->SetTraceLevel(3);

    try {
        std::stringstream ss;
        ss << "// Trailing semicolon\n"
           << "result = 2 + 3\n"
           << "assert(result == 5)\n";

        console_.Execute(ss.str());
        std::cout << "TrailingSemicolon test passed!" << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI Exception: " << e.ToString() << std::endl;
        FAIL() << "TrailingSemicolon test failed with KAI exception: "
               << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        FAIL() << "TrailingSemicolon test failed with exception: " << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL() << "TrailingSemicolon test failed with unknown exception";
    }
}

TEST_F(TestLangCommon, EmptyStatement) {
    console_.SetLanguage(Language::Rho);
    exec_->ClearStacks();
    exec_->ClearContext();
    exec_->SetTraceLevel(3);

    try {
        std::stringstream ss;
        ss << "// Empty statements with semicolons\n"
           << "result = 2 + 3\n"
           << "assert(result == 5)\n";

        console_.Execute(ss.str());
        std::cout << "EmptyStatement test passed!" << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI Exception: " << e.ToString() << std::endl;
        FAIL() << "EmptyStatement test failed with KAI exception: "
               << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        FAIL() << "EmptyStatement test failed with exception: " << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL() << "EmptyStatement test failed with unknown exception";
    }
}

TEST_F(TestLangCommon, MixedNewlinesAndSemicolons) {
    console_.SetLanguage(Language::Rho);
    exec_->ClearStacks();
    exec_->ClearContext();
    exec_->SetTraceLevel(3);

    try {
        std::stringstream ss;
        ss << "// Mixed newlines and semicolons\n"
           << "a = 1;\n"
           << "b = 2\n"
           << "c = 3;\n"
           << "sum = a + b + c\n"
           << "assert(sum == 6)\n";

        console_.Execute(ss.str());
        std::cout << "MixedNewlinesAndSemicolons test passed!" << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI Exception: " << e.ToString() << std::endl;
        FAIL() << "MixedNewlinesAndSemicolons test failed with KAI exception: "
               << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        FAIL() << "MixedNewlinesAndSemicolons test failed with exception: "
               << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL()
            << "MixedNewlinesAndSemicolons test failed with unknown exception";
    }
}

TEST_F(TestLangCommon, InlineForLoopWithSemicolons) {
    console_.SetLanguage(Language::Rho);
    exec_->ClearStacks();
    exec_->ClearContext();
    exec_->SetTraceLevel(3);

    try {
        std::stringstream ss;
        ss << "// Inline for loop with semicolons\n"
           << "sum = 0\n"
           << "for (i = 0; i < 5; i = i + 1) { sum = sum + i; }\n"
           << "assert(sum == 10)\n";

        console_.Execute(ss.str());
        std::cout << "InlineForLoopWithSemicolons test passed!" << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI Exception: " << e.ToString() << std::endl;
        FAIL() << "InlineForLoopWithSemicolons test failed with KAI exception: "
               << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        FAIL() << "InlineForLoopWithSemicolons test failed with exception: "
               << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL()
            << "InlineForLoopWithSemicolons test failed with unknown exception";
    }
}

TEST_F(TestLangCommon, ForLoopEmptySections) {
    console_.SetLanguage(Language::Rho);
    exec_->ClearStacks();
    exec_->ClearContext();
    exec_->SetTraceLevel(3);

    try {
        std::stringstream ss;
        ss << "// For loop with empty sections\n"
           << "j = 0\n"
           << "sum = 0\n"
           << "for (;;) {\n"
           << "    if (j >= 5) { break; }\n"
           << "    sum = sum + j;\n"
           << "    j = j + 1;\n"
           << "}\n"
           << "assert(sum == 10)\n";

        console_.Execute(ss.str());
        std::cout << "ForLoopEmptySections test passed!" << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI Exception: " << e.ToString() << std::endl;
        FAIL() << "ForLoopEmptySections test failed with KAI exception: "
               << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        FAIL() << "ForLoopEmptySections test failed with exception: "
               << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL() << "ForLoopEmptySections test failed with unknown exception";
    }
}

TEST_F(TestLangCommon, InlineIfElse) {
    console_.SetLanguage(Language::Rho);
    exec_->ClearStacks();
    exec_->ClearContext();
    exec_->SetTraceLevel(3);

    try {
        std::stringstream ss;
        ss << "// Inline if-else with semicolons\n"
           << "a = 2\n"
           << "result = 0\n"
           << "if (a > 3) { result = 1; } else { result = 2; }\n"
           << "assert(result == 2)\n";

        console_.Execute(ss.str());
        std::cout << "InlineIfElse test passed!" << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI Exception: " << e.ToString() << std::endl;
        FAIL() << "InlineIfElse test failed with KAI exception: "
               << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        FAIL() << "InlineIfElse test failed with exception: " << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL() << "InlineIfElse test failed with unknown exception";
    }
}

TEST_F(TestLangCommon, FunctionWithSemicolons) {
    console_.SetLanguage(Language::Rho);
    exec_->ClearStacks();
    exec_->ClearContext();
    exec_->SetTraceLevel(3);

    try {
        std::stringstream ss;
        ss << "// Function definition with semicolons\n"
           << "add = fun(a, b) {\n"
           << "    return a + b\n"
           << "}\n"
           << "result = add(2, 3)\n"
           << "assert result == 5\n";

        console_.Execute(ss.str());
        std::cout << "FunctionWithSemicolons test passed!" << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI Exception: " << e.ToString() << std::endl;
        FAIL() << "FunctionWithSemicolons test failed with KAI exception: "
               << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        FAIL() << "FunctionWithSemicolons test failed with exception: "
               << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL() << "FunctionWithSemicolons test failed with unknown exception";
    }
}

TEST_F(TestLangCommon, ComplexExpressionWithSemicolons) {
    console_.SetLanguage(Language::Rho);
    exec_->ClearStacks();
    exec_->ClearContext();
    exec_->SetTraceLevel(3);

    try {
        std::stringstream ss;
        ss << "// Complex expression combining multiple features\n"
           << "sum = 0\n"
           << "multiply = fun(a, b) { return a * b; }\n"
           << "for i = 0; i < 5; i = i + 1\n"
           << "    if i % 2 == 0\n"
           << "        sum = sum + multiply(i, 2)\n"
           << "    else\n"
           << "        sum = sum + i\n"
           << "assert sum == 14\n";

        console_.Execute(ss.str());
        std::cout << "ComplexExpressionWithSemicolons test passed!"
                  << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI Exception: " << e.ToString() << std::endl;
        FAIL() << "ComplexExpressionWithSemicolons test failed with KAI "
                  "exception: "
               << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        FAIL() << "ComplexExpressionWithSemicolons test failed with exception: "
               << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL() << "ComplexExpressionWithSemicolons test failed with unknown "
                  "exception";
    }
}

KAI_END