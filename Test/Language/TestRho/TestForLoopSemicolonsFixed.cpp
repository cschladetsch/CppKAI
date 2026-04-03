#include <iostream>
#include <sstream>

#include "../../Include/TestLangCommon.h"

KAI_BEGIN

// Test the Rho for loop with semicolons using correct Rho syntax (no
// parentheses)
TEST_F(TestLangCommon, ForLoopSemicolonsSyntaxFixed) {
    console_.SetLanguage(Language::Rho);

    // Clear stacks before testing
    exec_->ClearStacks();
    exec_->ClearContext();

    // Set trace level to debug
    exec_->SetTraceLevel(3);

    try {
        // Create a script with various for loop syntaxes that use semicolons
        std::stringstream ss;
        ss << "// For loop with standard semicolon syntax (no parentheses)\n"
           << "sum = 0\n"
           << "for i = 0; i < 5; i = i + 1\n"
           << "    sum = sum + i\n"
           << "assert(sum == 10)\n"
           << "\n"
           << "// For loop with empty initialization\n"
           << "count = 0\n"
           << "i = 0\n"
           << "for ; i < 3; i = i + 1\n"
           << "    count = count + 1\n"
           << "assert(count == 3)\n"
           << "\n"
           << "// For loop with empty condition section\n"
           << "count = 0\n"
           << "for i = 0; ; i = i + 1\n"
           << "    count = count + 1\n"
           << "    if count >= 3\n"
           << "        break\n"
           << "assert(count == 3)\n"
           << "\n"
           << "// For loop with empty increment section\n"
           << "sum = 0\n"
           << "for i = 0; i < 3;\n"
           << "    sum = sum + i\n"
           << "    i = i + 1\n"
           << "assert(sum == 3)\n"
           << "\n"
           << "// For loop with all sections empty\n"
           << "count = 0\n"
           << "for ;;\n"
           << "    count = count + 1\n"
           << "    if count >= 3\n"
           << "        break\n"
           << "assert(count == 3)\n"
           << "\n"
           << "// For loop with complex expressions\n"
           << "result = 0\n"
           << "add_one = fun(x)\n"
           << "    return x + 1\n"
           << "for i = add_one(0); i < add_one(2); i = add_one(i)\n"
           << "    result = result + i\n"
           << "assert(result == 3)\n";

        std::string script = ss.str();
        std::cout << "Script:\n" << script << std::endl;

        // Execute the script
        std::cout << "Executing script..." << std::endl;
        console_.Execute(script);

        // Success if we get here (all assertions passed)
        std::cout << "ForLoopSemicolonsSyntaxFixed test passed!" << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI Exception: " << e.ToString() << std::endl;
        FAIL()
            << "ForLoopSemicolonsSyntaxFixed test failed with KAI exception: "
            << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        FAIL() << "ForLoopSemicolonsSyntaxFixed test failed with exception: "
               << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL() << "ForLoopSemicolonsSyntaxFixed test failed with unknown "
                  "exception";
    }
}

// Test for loop with brace syntax (if supported)
TEST_F(TestLangCommon, ForLoopBraceSyntax) {
    console_.SetLanguage(Language::Rho);

    // Clear stacks before testing
    exec_->ClearStacks();
    exec_->ClearContext();

    // Set trace level to debug
    exec_->SetTraceLevel(3);

    try {
        // Create a script with for loops using brace syntax
        std::stringstream ss;
        ss << "// For loop with braces\n"
           << "sum = 0\n"
           << "for i = 0; i < 5; i = i + 1 {\n"
           << "    sum = sum + i\n"
           << "}\n"
           << "assert(sum == 10)\n"
           << "\n"
           << "// Nested for loops with braces\n"
           << "total = 0\n"
           << "for i = 0; i < 2; i = i + 1 {\n"
           << "    for j = 0; j < 2; j = j + 1 {\n"
           << "        total = total + (i * 2 + j)\n"
           << "    }\n"
           << "}\n"
           << "assert(total == 6)\n";

        std::string script = ss.str();
        std::cout << "Script:\n" << script << std::endl;

        // Execute the script
        std::cout << "Executing script..." << std::endl;
        console_.Execute(script);

        // Success if we get here (all assertions passed)
        std::cout << "ForLoopBraceSyntax test passed!" << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI Exception: " << e.ToString() << std::endl;
        FAIL() << "ForLoopBraceSyntax test failed with KAI exception: "
               << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        FAIL() << "ForLoopBraceSyntax test failed with exception: " << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL() << "ForLoopBraceSyntax test failed with unknown exception";
    }
}

KAI_END