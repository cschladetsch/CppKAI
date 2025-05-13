#include <iostream>
#include <sstream>

#include "../../Include/TestLangCommon.h"

KAI_BEGIN

// Test the Rho for loop with semicolons
TEST_F(TestLangCommon, ForLoopSemicolonsSyntax) {
    _console.SetLanguage(Language::Rho);

    // Clear stacks before testing
    _exec->ClearStacks();
    _exec->ClearContext();

    // Set trace level to debug
    _exec->SetTraceLevel(3);

    try {
        // Create a script with various for loop syntaxes that use semicolons
        std::stringstream ss;
        ss << "// For loop with standard semicolon syntax\n"
           << "sum = 0\n"
           << "for (i = 0; i < 5; i = i + 1)\n"
           << "    sum = sum + i\n"
           << "assert(sum == 10)\n"
           << "\n"
           << "// For loop with multiple initializations using comma\n"
           << "product = 0\n"
           << "for (i = 1, j = 10; i <= 3; i = i + 1)\n"
           << "    product = i * j\n"
           << "assert(product == 30)\n"
           << "\n"
           << "// For loop with multiple increments using comma\n"
           << "sum = 0\n"
           << "for (i = 0, j = 5; i < 3; i = i + 1, j = j - 1)\n"
           << "    sum = sum + i + j\n"
           << "assert(sum == 15)\n"
           << "\n"
           << "// For loop with empty sections but semicolons present\n"
           << "count = 0\n"
           << "i = 0\n"
           << "for (; i < 3; i = i + 1)\n"
           << "    count = count + 1\n"
           << "assert(count == 3)\n"
           << "\n"
           << "// For loop with empty condition section\n"
           << "count = 0\n"
           << "for (i = 0; ; i = i + 1)\n"
           << "    count = count + 1\n"
           << "    if (count >= 3)\n"
           << "        break\n"
           << "assert(count == 3)\n"
           << "\n"
           << "// For loop with empty increment section\n"
           << "sum = 0\n"
           << "for (i = 0; i < 3; )\n"
           << "    sum = sum + i\n"
           << "    i = i + 1\n"
           << "assert(sum == 3)\n"
           << "\n"
           << "// For loop with all sections empty\n"
           << "count = 0\n"
           << "for (;;)\n"
           << "    count = count + 1\n"
           << "    if (count >= 3)\n"
           << "        break\n"
           << "assert(count == 3)\n"
           << "\n"
           << "// For loop with complex expressions\n"
           << "result = 0\n"
           << "add_one = fun(x) { return x + 1 }\n"
           << "for (i = add_one(0); i < add_one(2); i = add_one(i))\n"
           << "    result = result + i\n"
           << "assert(result == 3)\n";

        std::string script = ss.str();
        std::cout << "Script:\n" << script << std::endl;

        // Execute the script
        std::cout << "Executing script..." << std::endl;
        _console.Execute(script);

        // Success if we get here (all assertions passed)
        std::cout << "ForLoopSemicolonsSyntax test passed!" << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI Exception: " << e.ToString() << std::endl;
        FAIL() << "ForLoopSemicolonsSyntax test failed with KAI exception: "
               << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        FAIL() << "ForLoopSemicolonsSyntax test failed with exception: "
               << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL() << "ForLoopSemicolonsSyntax test failed with unknown exception";
    }
}

// Test advanced for loop syntax with formatting variations
TEST_F(TestLangCommon, ForLoopFormattingVariations) {
    _console.SetLanguage(Language::Rho);

    // Clear stacks before testing
    _exec->ClearStacks();
    _exec->ClearContext();

    // Set trace level to debug
    _exec->SetTraceLevel(3);

    try {
        // Create a script with variations in for loop formatting
        std::stringstream ss;
        ss << "// For loop with whitespace around semicolons\n"
           << "count = 0\n"
           << "for (i = 0 ; i < 3 ; i = i + 1)\n"
           << "    count = count + 1\n"
           << "assert(count == 3)\n"
           << "\n"
           << "// For loop with break and continue\n"
           << "sum = 0\n"
           << "for (i = 0; i < 10; i = i + 1)\n"
           << "    if (i % 2 != 0)\n"
           << "        continue\n"
           << "    sum = sum + i\n"
           << "    if (i >= 6)\n"
           << "        break\n"
           << "assert(sum == 12)\n"
           << "\n"
           << "// Nested for loops\n"
           << "total = 0\n"
           << "for (i = 0; i < 2; i = i + 1)\n"
           << "    for (j = 0; j < 2; j = j + 1)\n"
           << "        total = total + (i * 2 + j)\n"
           << "assert(total == 6)\n";

        std::string script = ss.str();
        std::cout << "Script:\n" << script << std::endl;

        // Execute the script
        std::cout << "Executing script..." << std::endl;
        _console.Execute(script);

        // Success if we get here (all assertions passed)
        std::cout << "ForLoopFormattingVariations test passed!" << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI Exception: " << e.ToString() << std::endl;
        FAIL() << "ForLoopFormattingVariations test failed with KAI exception: "
               << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        FAIL() << "ForLoopFormattingVariations test failed with exception: "
               << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL()
            << "ForLoopFormattingVariations test failed with unknown exception";
    }
}

KAI_END